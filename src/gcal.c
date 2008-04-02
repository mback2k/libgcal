/**
 * @file   gcal.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 12:32:11 2008
 *
 * @brief  Base file for a gcalendar service access library.
 *
 * \todo:
 * - parse Atom feed (events and calendar lists)
 * - enable user get all events
 * - enable user access events individually
 * - enable user do queries: by string and date range
 * - enable user add new events
 * - enable user edit events
 * - enable user delete events
 * - enable user list and access available calendars
 * - enable user create new calendars
 * - enable user delete calendars
 * - think a way to securely store passwords
 * - more utests
 * - should we use a logging of some sort?
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "gcal.h"
#include "gcal_parser.h"

static const char GCAL_URL[] = "https://www.google.com/accounts/ClientLogin";
static const char GCAL_LIST[] = "http://www.google.com/calendar/feeds/"
	"default/allcalendars/full";
static const char GCAL_EVENT_START[] = "http://www.google.com/calendar/feeds/";
static const char GCAL_EVENT_END[] = "@gmail.com/private/full";


static const int GCAL_DEFAULT_ANSWER = 200;
static const int GCAL_EVENT_ANSWER = 302;
static const char EMAIL_FIELD[] = "Email=";
static const char EMAIL_ADDRESS[] = "@gmail.com";
static const char PASSWD_FIELD[] = "Passwd=";
static const char TRAILING_FIELD[] = "service=cl&source=libgcal";
static const char HEADER_BREAK = '\n';
static const char HEADER_GET[] = "Authorization: GoogleLogin auth=";

/** Library structure. It holds resources (curl, buffer, etc).
 */
struct gcal_resource {
	/** Memory buffer */
	char *buffer;
	/** Its length */
	size_t length;
	/** gcalendar authorization */
	char *auth;
	/** curl data structure */
	CURL *curl;
	/** Atom feed URL */
	char *url;
	/** The user name */
	char *user;
	/** DOM xml tree (an abstract type so I can plug another xml parser) */
	dom_document *document;
	/** A flag to control if the buffer has XML atom stream */
	char has_xml;

};

static void reset_buffer(struct gcal_resource *ptr)
{
	if (ptr->buffer)
		free(ptr->buffer);
	ptr->length = 256;
	ptr->buffer = (char *) calloc(ptr->length, sizeof(char));
}

struct gcal_resource *gcal_initialize(void)
{

	struct gcal_resource *ptr;
	ptr = (struct gcal_resource *) malloc(sizeof(struct gcal_resource));
	if (!ptr) {
		fprintf(stderr, "%s\n", "gcal_initialize: failed malloc\n");
		goto exit;
	}

	ptr->has_xml = 0;
	ptr->document = NULL;
	ptr->user = NULL;
	ptr->url = NULL;
	ptr->auth = NULL;
	ptr->buffer = NULL;
	reset_buffer(ptr);
	ptr->curl = curl_easy_init();

	if (!(ptr->buffer) || (!(ptr->curl))) {
		gcal_destroy(ptr);
		ptr = NULL;
	}

exit:
	return ptr;
}

static void clean_buffer(struct gcal_resource *gcal_obj)
{
	memset(gcal_obj->buffer, 0, gcal_obj->length);
}

void gcal_destroy(struct gcal_resource *gcal_obj)
{

	if (gcal_obj->buffer)
		free(gcal_obj->buffer);
	if (gcal_obj->curl)
		curl_easy_cleanup(gcal_obj->curl);
	if (gcal_obj->auth)
		free(gcal_obj->auth);
	if (gcal_obj->url)
		free(gcal_obj->url);
	if (gcal_obj->user)
		free(gcal_obj->user);
	if (gcal_obj->document)
		clean_dom_document(gcal_obj->document);

}


static size_t write_cb(void *ptr, size_t count, size_t chunk_size, void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *) data;
	int current_length = strnlen(gcal_ptr->buffer, gcal_ptr->length);

	if (size > (gcal_ptr->length - current_length - 1)) {
		    gcal_ptr->length = current_length + size + 1;
		    /* FIXME: is it save to continue reallocing more memory?
		     * what happens if the gcalendar list is *really* big?
		     * how big can it be? Maybe I should use another write
		     * callback
		     * when requesting the Atom feed (one that will treat the
		     * the stream as its being read and not store it in memory).
		     */
		    gcal_ptr->buffer = realloc(gcal_ptr->buffer,
					       gcal_ptr->length);

		    if (!gcal_ptr->buffer) {
			    fprintf(stderr, "write_cb: Failed relloc\n");
			    goto exit;
		    }

	}

	strncat(gcal_ptr->buffer, (char *)ptr, size);

exit:
	return size;
}

static int check_request_error(struct gcal_resource *ptr_gcal, int code,
			       int expected_answer)
{
	long request_stat;
	int result = 0;

	curl_easy_getinfo(ptr_gcal->curl, CURLINFO_HTTP_CODE, &request_stat);
	if (code || (request_stat != expected_answer)) {
		fprintf(stderr, "%s\n%s%s\n%s%d\n",
			"gcal_get_authentication: failed request.",
			"Curl code: ", curl_easy_strerror(code),
			"HTTP code: ", (int)request_stat);
		result = -1;
	}

	return result;
}


int gcal_get_authentication(char *user, char *password,
			    struct gcal_resource *ptr_gcal)
{
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	int post_len = 0;
	char *post = NULL;
	int result = -1;

	int count = 0;
	char *ptr = NULL;


	if (!user || !password)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	ptr_gcal->user = strdup(user);
	post_len = strlen(user) + strlen(password) +
		sizeof(EMAIL_FIELD) + sizeof(EMAIL_ADDRESS) +
		sizeof(PASSWD_FIELD) + sizeof(TRAILING_FIELD);
	post = (char *) malloc(post_len);
	if (!post || !ptr_gcal->user)
		goto exit;

	snprintf(post, post_len - 1, "%s%s%s&%s%s&%s",
		 EMAIL_FIELD, user, EMAIL_ADDRESS,
		 PASSWD_FIELD, password, TRAILING_FIELD);


	response_headers = curl_slist_append(response_headers,
					     "application/x-www-form-urlencoded"
					     );
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, GCAL_URL);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_POSTFIELDS, post);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEDATA, (void *)ptr_gcal);

	res = curl_easy_perform(ptr_gcal->curl);
	if (check_request_error(ptr_gcal, res, GCAL_DEFAULT_ANSWER))
		goto cleanup;

	/* gcalendar server returns a string like this:
	 * SID=value\n
	 * LSID=value\n
	 * Auth=value\n
	 * and we only need the authorization token to login later.
	 * TODO: move this to a distinct function and write utests.
	 */
	if (ptr_gcal->auth)
		free(ptr_gcal->auth);
	ptr = ptr_gcal->buffer;
	while ((ptr = strchr(ptr, HEADER_BREAK))) {
		++count;
		++ptr;
		if (count == 2) {
			ptr_gcal->auth = strdup(ptr + sizeof("Auth"));
			if (!ptr_gcal->auth)
				goto cleanup;
		}

	}

	result = 0;

cleanup:
	if (post)
		free(post);
	if (response_headers)
		curl_slist_free_all(response_headers);

exit:
	return result;

}

static int get_follow_redirection(struct gcal_resource *ptr_gcal,
				  const char *url)
{
	struct curl_slist *response_headers = NULL;
	int length = 0;
	int result = -1;
	char *tmp_buffer = NULL;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	length = strlen(ptr_gcal->auth) + sizeof(HEADER_GET) + 1;
	tmp_buffer = (char *) malloc(length);
	if (!tmp_buffer)
		goto exit;
	snprintf(tmp_buffer, length - 1, "%s%s", HEADER_GET, ptr_gcal->auth);

	response_headers = curl_slist_append(response_headers, tmp_buffer);

	curl_easy_setopt(ptr_gcal->curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, url);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEDATA, (void *)ptr_gcal);

	result = curl_easy_perform(ptr_gcal->curl);
	if (check_request_error(ptr_gcal, result, GCAL_EVENT_ANSWER)) {
		result = -1;
		goto cleanup;
	}

	/* It will extract and follow the first 'REF' link in the stream */
	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url)) {
		result = -1;
		goto cleanup;
	}

	clean_buffer(ptr_gcal);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, ptr_gcal->url);
	result = curl_easy_perform(ptr_gcal->curl);
	if ((result = check_request_error(ptr_gcal, result,
					  GCAL_DEFAULT_ANSWER))) {
		result = -1;
		goto cleanup;
	}

cleanup:

	if (tmp_buffer)
		free(tmp_buffer);
	if (response_headers)
		curl_slist_free_all(response_headers);

exit:
	return result;
}

int gcal_dump(struct gcal_resource *ptr_gcal)
{
	int result = 0;
	char *buffer = NULL;
	int length = 0;

	length = sizeof(GCAL_EVENT_START) + sizeof(GCAL_EVENT_END) +
		strlen(ptr_gcal->user) + 1;
	buffer = (char *)malloc(length);
	if (!buffer)
		goto exit;

	snprintf(buffer, length - 1, "%s%s%s", GCAL_EVENT_START, ptr_gcal->user,
		 GCAL_EVENT_END);
	result =  get_follow_redirection(ptr_gcal, buffer);

	if (!result)
		ptr_gcal->has_xml = 1;

	free(buffer);
exit:

	return result;
}

int gcal_calendar_list(struct gcal_resource *ptr_gcal)
{
	int result;
	result =  get_follow_redirection(ptr_gcal, GCAL_LIST);
	/* TODO: parse the Atom feed */

	return result;
}

int gcal_entries_number(struct gcal_resource *ptr_gcal)
{
	int result = -1;

	if (!ptr_gcal)
		goto exit;

	if (!ptr_gcal->buffer || !ptr_gcal->has_xml)
		goto exit;

	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto exit;

	result = get_entries_number(ptr_gcal->document);
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

exit:
	return result;
}

struct gcal_entries *gcal_get_entries(struct gcal_resource *ptr_gcal,
				      size_t *length)
{

	int result = -1;
	struct gcal_entries *ptr_res = NULL;

	if (!ptr_gcal)
		goto exit;

	if (!ptr_gcal->buffer || !ptr_gcal->has_xml)
		goto exit;

	/* create a doc and free atom xml buffer
	 * TODO: I'm not completely sure if reseting the buffer
	 * is a good idea (say that the user wants to do something else
	 * using the atom stream?).
	 */
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto exit;
	reset_buffer(ptr_gcal);

	result = get_entries_number(ptr_gcal->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_entries) * result);
	if (!ptr_res)
		goto cleanup;

	*length = result;
	result = extract_all_entries(ptr_gcal->document, ptr_res, result);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

	goto exit;

cleanup:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

exit:

	return ptr_res;
}


static void clean_string(char *ptr_str)
{
	if (ptr_str)
		free(ptr_str);
}

void gcal_destroy_entry(struct gcal_entries *entry)
{
	if (!entry)
		return;

	clean_string(entry->title);
	clean_string(entry->id);
	clean_string(entry->edit_uri);
	clean_string(entry->content);
	clean_string(entry->dt_recurrent);
	clean_string(entry->dt_start);
	clean_string(entry->dt_end);
	clean_string(entry->where);
	clean_string(entry->status);
	clean_string(entry->updated);
}

void gcal_destroy_entries(struct gcal_entries *entries, size_t length)
{
	size_t i = 0;
	if (!entries)
		return;

	for (; i < length; ++i)
		gcal_destroy_entry((entries + i));

	free(entries);
}

