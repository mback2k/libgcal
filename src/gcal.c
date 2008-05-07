/*
Copyright (c) 2008 Instituto Nokia de Tecnologia
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the INdT nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @file   gcal.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 12:32:11 2008
 *
 * @brief  Base file for a gcalendar service access library.
 *
 * \todo:
 * - enable user edit events
 * - enable user delete events
 * - enable user do queries: by string and date range
 * - enable user list and access available calendars
 * - enable user create new calendars
 * - enable user delete calendars
 * - handle http_proxy
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
static const char GCAL_EDIT_URL[] = "http://www.google.com/calendar/feeds"
	"/default/private/full";
static const char GCAL_EVENT_START[] = "http://www.google.com/calendar/feeds/";
static const char GCAL_EVENT_END[] = "@gmail.com/private/full";


static const int GCAL_DEFAULT_ANSWER = 200;
static const int GCAL_REDIRECT_ANSWER = 302;
static const int GCAL_EDIT_ANSWER = 201;

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

static int check_request_error(CURL *curl_ctx, int code,
			       int expected_answer)
{
	long request_stat;
	int result = 0;

	curl_easy_getinfo(curl_ctx, CURLINFO_HTTP_CODE, &request_stat);
	if (code || (request_stat != expected_answer)) {
		fprintf(stderr, "%s\n%s%s\n%s%d\n",
			"gcal_get_authentication: failed request.",
			"Curl code: ", curl_easy_strerror(code),
			"HTTP code: ", (int)request_stat);
		result = -1;
	}

	return result;
}


static int http_post(struct gcal_resource *ptr_gcal, const char *url,
		     char *header, char *header2, char *header3, char *post_data,
		     const int expected_answer)
{
	int result = -1;
	CURLcode res;
	CURL *curl_ctx = ptr_gcal->curl;
	struct curl_slist *response_headers = NULL;

	if (header)
		response_headers = curl_slist_append(response_headers, header);
	if (header2)
		response_headers = curl_slist_append(response_headers, header2);
	if (header3)
		response_headers = curl_slist_append(response_headers, header3);

	if (!response_headers)
		return result;

	curl_easy_setopt(curl_ctx, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(curl_ctx, CURLOPT_POST, 1);
	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, strlen(post_data));
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, (void *)ptr_gcal);

	res = curl_easy_perform(curl_ctx);
	result = check_request_error(ptr_gcal->curl, res, expected_answer);
	curl_slist_free_all(response_headers);

	return result;

}


int gcal_get_authentication(char *user, char *password,
			    struct gcal_resource *ptr_gcal)
{

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


	result = http_post(ptr_gcal, GCAL_URL,
			   "Content-Type: application/x-www-form-urlencoded",
			   NULL, NULL, post, GCAL_DEFAULT_ANSWER);
	if (result)
		goto cleanup;

	/* gcalendar server returns a string like this:
	 * SID=value\n
	 * LSID=value\n
	 * Auth=value\n
	 * and we only need the authorization token to login later
	 * without the '\r\n' in the end of string.
	 * TODO: move this to a distinct function and write utests.
	 */
	if (ptr_gcal->auth)
		free(ptr_gcal->auth);
	ptr = ptr_gcal->buffer;
	ptr[strlen(ptr) - 1] = '\0';
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
	if (check_request_error(ptr_gcal->curl, result, GCAL_REDIRECT_ANSWER)) {
		result = -1;
		goto cleanup;
	}

	/* It will extract and follow the first 'REF' link in the stream */
	if (ptr_gcal->url)
		free(ptr_gcal->url);
	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url)) {
		result = -1;
		goto cleanup;
	}

	clean_buffer(ptr_gcal);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, ptr_gcal->url);
	result = curl_easy_perform(ptr_gcal->curl);
	if ((result = check_request_error(ptr_gcal->curl, result,
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
	int result = -1;
	char *buffer = NULL;
	int length = 0;

	if (!ptr_gcal)
		goto exit;
	/* Failed to get authentication token */
	if (!ptr_gcal->auth)
		goto exit;

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
	/* Failed to get authentication token */
	if (!ptr_gcal->auth)
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

int gcal_create_event(struct gcal_entries *entries,
		      struct gcal_resource *ptr_gcal)
{
	int result = -1;
	int length = 0;
	char *h_auth = NULL, *h_length = NULL, *xml_entry = NULL, *tmp;
	const char header[] = "Content-length: ";

	if (!entries || !ptr_gcal)
		goto exit;

	if (!ptr_gcal->auth)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	/* mount XML entry */
	result = xmlentry_create(entries, &xml_entry, &length);
	if (result == -1)
		goto cleanup;


	/* Mounts content length and  authentication header strings */
	length = strlen(xml_entry) + strlen(header) + 1;
	h_length = (char *) malloc(length) ;
	if (!h_length)
		goto exit;
	strncpy(h_length, header, sizeof(header));
	tmp = h_length + sizeof(header) - 1;
	snprintf(tmp, length - (sizeof(header) + 1), "%d", strlen(xml_entry));


	length = strlen(ptr_gcal->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, ptr_gcal->auth);


	/* Post the entry data */
	result = http_post(ptr_gcal, GCAL_EDIT_URL,
			   "Content-Type: application/atom+xml",
			   h_length,
			   h_auth,
			   xml_entry, GCAL_REDIRECT_ANSWER);
	if (result == -1)
		goto cleanup;

	if (ptr_gcal->url)
		free(ptr_gcal->url);
	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url))
		goto cleanup;

	/* fprintf(stderr, "result = %s\n", ptr_gcal->buffer); */
	clean_buffer(ptr_gcal);

	/* Add gsessionid to post URL */
	result = http_post(ptr_gcal, ptr_gcal->url,
			   "Content-Type: application/atom+xml",
			   h_length,
			   h_auth,
			   xml_entry, GCAL_EDIT_ANSWER);

	if (result == -1) {
		fprintf(stderr, "result = %s\n", ptr_gcal->buffer);
		fprintf(stderr, "\nurl = %s\nh_length = %s\nh_auth = %s"
			"\nxml_entry =%s%d\n",
			ptr_gcal->url, h_length, h_auth, xml_entry,
			strlen(xml_entry));
		goto cleanup;
	}

cleanup:

	if (xml_entry)
		free(xml_entry);
	if (h_length)
		free(h_length);
	if (h_auth)
		free(h_auth);

exit:
	return result;
}

int gcal_delete_event(struct gcal_entries *entry,
		      struct gcal_resource *ptr_gcal)
{
	int result = -1, length;
	char *h_auth;

	if (!entry || !ptr_gcal)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	length = strlen(ptr_gcal->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, ptr_gcal->auth);

	fprintf(stderr, "Before HTTP request!");
	result = http_post(ptr_gcal, entry->edit_uri,
			   "Content-Type: application/atom+xml",
			   NULL,
			   h_auth,
			   "DELETE", GCAL_DEFAULT_ANSWER);

/* 	fprintf(stderr, "result = %s\nuri = %s\n", ptr_gcal->buffer, */
/* 		entry->edit_uri); */

exit:

	return result;

}
