/**
 * @file   gcal.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 12:32:11 2008
 *
 * @brief  Base file for a gcalendar service access library.
 *
 * \todo:
 * - write methods to access gcalendar events
 * - think a way to securely store passwords
 * - more utests
 * - should we use a logging of some sort?
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "gcal.h"

static const char GCAL_URL[] = "https://www.google.com/accounts/ClientLogin";
static const char GCAL_EVENTS[] = "http://www.google.com/calendar/feeds/default"
	"/owncalendars/full";
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
};

struct gcal_resource *gcal_initialize(void)
{

	struct gcal_resource *ptr;
	ptr = (struct gcal_resource *) malloc(sizeof(struct gcal_resource));
	if (!ptr) {
		fprintf(stderr, "%s\n", "gcal_initialize: failed malloc\n");
		goto exit;
	}

	ptr->url = NULL;
	ptr->auth = NULL;
	ptr->length = 256;
	ptr->buffer = (char *) calloc(ptr->length, sizeof(char));
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

}


static size_t write_cb(void *ptr, size_t count, size_t chunk_size, void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *) data;

	if (size > (gcal_ptr->length -
		    strnlen(gcal_ptr->buffer, gcal_ptr->length) - 1)) {
		    gcal_ptr->length += size + 1;
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

	strncat(gcal_ptr->buffer, (char *)ptr, gcal_ptr->length);

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

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	post_len = strlen(user) + strlen(password) +
		sizeof(EMAIL_FIELD) + sizeof(EMAIL_ADDRESS) +
		sizeof(PASSWD_FIELD) + sizeof(TRAILING_FIELD);

	if (!(post = (char *) malloc(post_len)))
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
	free(post);

exit:
	return result;

}

/* REMARK: this function is recursive, I'm not completely sure if this
 * is a good idea (i.e. for small devices).
 */
static char *get(xmlNode *a_node)
{
	xmlNode *cur_node = NULL;
	char *result = NULL;
	xmlChar *uri = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (xmlHasProp(cur_node, "HREF")) {
			uri = xmlGetProp(cur_node, "HREF");
			if (uri) {
				result = strdup(uri);
				xmlFree(uri);
				goto exit;
			}

		}

		result = get(cur_node->children);
		if (result)
			goto exit;
	}

exit:
	return result;

}

int get_the_url(char *data, int length, char **url)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	int result = -1;

	*url = NULL;
	doc = xmlReadMemory(data, length, "noname.xml", NULL, 0);
	if (!doc)
		goto exit;

	root_element = xmlDocGetRootElement(doc);
	*url = get(root_element);

	xmlFreeDoc(doc);
	xmlCleanupParser();
	result = 0;
exit:
	return result;
}

int gcal_dump(struct gcal_resource *ptr_gcal)
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
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, GCAL_EVENTS);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEDATA, (void *)ptr_gcal);

	result = curl_easy_perform(ptr_gcal->curl);
	if (check_request_error(ptr_gcal, result, GCAL_EVENT_ANSWER)) {
		result = -1;
		goto cleanup;
	}

	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url)) {
		result = -1;
		goto cleanup;
	}

	clean_buffer(ptr_gcal);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, ptr_gcal->url);
	result = curl_easy_perform(ptr_gcal->curl);
	if (check_request_error(ptr_gcal, result, GCAL_DEFAULT_ANSWER)) {
		result = -1;
		goto cleanup;
	}

	/* TODO: get all the Atom feed and parse its XML */
 	/* printf("%s\n", ptr_gcal->buffer); */

cleanup:

	free(tmp_buffer);
exit:
	return result;
}
