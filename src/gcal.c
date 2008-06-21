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
 * High priority
 * - enable user do queries: by string and date range
 * - batch operation (add/edit/delete): will require some new public functions
 * - handle http_proxy
 *
 * Lower priority
 * - enable user list and access available calendars
 * - enable user create new calendars
 * - enable user delete calendars
 * - think a way to securely store passwords
 * - more utests
 * - provide option to use another XML parser (maybe expat?)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "internal_gcal.h"
#include "gcal.h"
#include "gcal_parser.h"

#ifdef GCAL_DEBUG_CURL
#include "curl_debug_gcal.h"
#endif

static void reset_buffer(struct gcal_resource *ptr)
{
	if (ptr->buffer)
		free(ptr->buffer);
	ptr->length = 256;
	ptr->buffer = (char *) calloc(ptr->length, sizeof(char));
}

struct gcal_resource *gcal_initialize(gservice mode)
{

	struct gcal_resource *ptr;
	ptr = (struct gcal_resource *) malloc(sizeof(struct gcal_resource));
	if (!ptr)
		goto exit;

	ptr->has_xml = 0;
	ptr->document = NULL;
	ptr->user = NULL;
	ptr->url = NULL;
	ptr->auth = NULL;
	ptr->buffer = NULL;
	reset_buffer(ptr);
	ptr->curl = curl_easy_init();
	ptr->http_code = 0;
	ptr->curl_msg = NULL;
	ptr->http_code = 0;
	ptr->internal_status = 0;
	ptr->fout_log = NULL;
	ptr->max_results = strdup(GCAL_UPPER);
	ptr->timezone = NULL;
	ptr->location = NULL;
	ptr->deleted = HIDE;

	if (!(ptr->buffer) || (!(ptr->curl)) || (!ptr->max_results)) {
		if (ptr->max_results)
			free(ptr->max_results);
		gcal_destroy(ptr);
		ptr = NULL;
		goto exit;
	}

	/* Initializes to google calendar as default */
	gcal_set_service(ptr, mode);

exit:
	return ptr;
}

void gcal_set_service(struct gcal_resource *ptr_gcal, gservice mode)
{

	if (ptr_gcal) {
		if (mode == GCALENDAR)
			strcpy(ptr_gcal->service, "cl");
		else if (mode == GCONTACT)
			strcpy(ptr_gcal->service, "cp");

	}
}

void clean_buffer(struct gcal_resource *gcal_obj)
{
	memset(gcal_obj->buffer, 0, gcal_obj->length);
}

void gcal_destroy(struct gcal_resource *gcal_obj)
{
	if (!gcal_obj)
		return;

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
	if (gcal_obj->curl_msg)
		free(gcal_obj->curl_msg);
	if (gcal_obj->fout_log)
		fclose(gcal_obj->fout_log);
	if (gcal_obj->max_results)
		free(gcal_obj->max_results);
	if (gcal_obj->timezone)
		free(gcal_obj->timezone);
	if (gcal_obj->location)
		free(gcal_obj->location);

	free(gcal_obj);
}


static size_t write_cb(void *ptr, size_t count, size_t chunk_size, void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *) data;
	int current_length = strnlen(gcal_ptr->buffer, gcal_ptr->length);
	char *ptr_tmp;

	if (size > (gcal_ptr->length - current_length - 1)) {
		gcal_ptr->length = current_length + size + 1;
		/* TODO: is it save to continue reallocing more memory?
		 * what happens if the gcalendar list is *really* big?
		 * how big can it be? Maybe I should use another write
		 * callback
		 * when requesting the Atom feed (one that will treat the
		 * the stream as its being read and not store it in memory).
		 */
		ptr_tmp = realloc(gcal_ptr->buffer, gcal_ptr->length);

		if (!ptr_tmp) {
			if (gcal_ptr->fout_log)
				fprintf(gcal_ptr->fout_log,
					"write_cb: Failed relloc!\n");
			goto exit;
		}

		gcal_ptr->buffer = ptr_tmp;
	}

	strncat(gcal_ptr->buffer, (char *)ptr, size);

exit:
	return size;
}

static int check_request_error(struct gcal_resource *ptr_gcal, int code,
			       int expected_answer)
{
	int result = 0;
	CURL *curl_ctx = ptr_gcal->curl;

	curl_easy_getinfo(curl_ctx, CURLINFO_HTTP_CODE,
			  &(ptr_gcal->http_code));
	if (code || (ptr_gcal->http_code != expected_answer)) {

		if (ptr_gcal->curl_msg)
			free(ptr_gcal->curl_msg);

		ptr_gcal->curl_msg = strdup(curl_easy_strerror(code));

		if (ptr_gcal->fout_log)
			fprintf(ptr_gcal->fout_log, "%s\n%s%s\n%s%d\n",
				"check_request_error: failed request.",
				"Curl code: ", ptr_gcal->curl_msg,
				"HTTP code: ", (int)ptr_gcal->http_code);
		result = -1;
	}

	return result;
}

static int common_upload(struct gcal_resource *ptr_gcal,
			 char *header, char *header2, char *header3,
			 struct curl_slist **curl_headers)
{
	int result = -1;
	CURL *curl_ctx = ptr_gcal->curl;
	struct curl_slist *response_headers = NULL;

#ifdef GCAL_DEBUG_CURL
	struct data_curl_debug flag;
	flag.trace_ascii = 1;
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_DEBUGFUNCTION,
			 curl_debug_gcal_trace);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_DEBUGDATA, &flag);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_VERBOSE, 1);
#endif

	if (header)
		response_headers = curl_slist_append(response_headers, header);
	if (header2)
		response_headers = curl_slist_append(response_headers, header2);
	if (header3)
		response_headers = curl_slist_append(response_headers, header3);

	if (!response_headers)
		return result;

	*curl_headers = response_headers;

	curl_easy_setopt(curl_ctx, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, (void *)ptr_gcal);

	return result = 0;
}

int http_post(struct gcal_resource *ptr_gcal, const char *url,
	      char *header, char *header2, char *header3,
	      char *post_data, const int expected_answer)
{
	int result = -1;
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	CURL *curl_ctx;
	if (!ptr_gcal)
		goto exit;

	curl_ctx = ptr_gcal->curl;
	result = common_upload(ptr_gcal, header, header2, header3,
			       &response_headers);
	if (result)
		goto exit;

	/* It seems deprecated, as long I set POSTFIELDS */
	curl_easy_setopt(curl_ctx, CURLOPT_POST, 1);
	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	if (post_data) {
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE,
				 strlen(post_data));
	}
	else
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, 0);

	res = curl_easy_perform(curl_ctx);
	result = check_request_error(ptr_gcal, res, expected_answer);

	/* cleanup */
	curl_slist_free_all(response_headers);

exit:
	return result;

}

static int http_put(struct gcal_resource *ptr_gcal, const char *url,
		    char *header, char *header2, char *header3,
		    char *post_data, const int expected_answer)
{
	int result = -1;
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	CURL *curl_ctx;
	if (!ptr_gcal)
		goto exit;

	curl_ctx = ptr_gcal->curl;
	result = common_upload(ptr_gcal, header, header2, header3,
			       &response_headers);
	if (result)
		goto exit;

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	/* Tells curl that I want to PUT */
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_CUSTOMREQUEST, "PUT");

	if (post_data) {
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE,
				 strlen(post_data));
	}
	else
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, 0);



	res = curl_easy_perform(curl_ctx);
	result = check_request_error(ptr_gcal, res, expected_answer);

	/* cleanup */
	curl_slist_free_all(response_headers);

exit:
	return result;

}

int gcal_get_authentication(struct gcal_resource *ptr_gcal,
			    char *user, char *password)
{

	int post_len = 0;
	char *post = NULL;
	int result = -1;
	char *tmp = NULL;

	if (!user || !password)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	ptr_gcal->user = strdup(user);
	post_len = strlen(user) + strlen(password) +
		sizeof(EMAIL_FIELD) + sizeof(EMAIL_ADDRESS) +
		sizeof(PASSWD_FIELD) + sizeof(SERVICE_FIELD) +
		strlen(ptr_gcal->service) + sizeof(CLIENT_SOURCE)
		+ 4; /* thanks to 3 '&' between fields + null character */
	post = (char *) malloc(post_len);
	if (!post || !ptr_gcal->user)
		goto exit;

	snprintf(post, post_len - 1,
		 "%s%s%s&"
		 "%s%s&"
		 "%s%s&"
		 "%s",
		 EMAIL_FIELD, user, EMAIL_ADDRESS,
		 PASSWD_FIELD, password,
		 SERVICE_FIELD, ptr_gcal->service,
		 CLIENT_SOURCE);

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

	ptr_gcal->auth = strstr(ptr_gcal->buffer, HEADER_AUTH);
	ptr_gcal->auth = strdup(ptr_gcal->auth + strlen(HEADER_AUTH));
	if (!ptr_gcal->auth)
		goto cleanup;

	tmp = strstr(ptr_gcal->auth, "\n");
	if (tmp)
		*tmp = '\0';

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

	if (!(strcmp(ptr_gcal->service, "cp"))) {
		/* For contacts, there is *not* redirection. */
		if (!(result = check_request_error(ptr_gcal, result,
						   GCAL_DEFAULT_ANSWER))) {
			result = 0;
			goto cleanup;
		}
	} else if (!(strcmp(ptr_gcal->service, "cl"))) {
		/* For calendar, it *must* be redirection */
		if (check_request_error(ptr_gcal, result,
					GCAL_REDIRECT_ANSWER)) {
			result = -1;
			goto cleanup;
		}
	} else {
		/* No valid service, just exit. */
		result = -1;
		goto cleanup;
	}

	/* It will extract and follow the first 'REF' link in the stream */
	if (ptr_gcal->url) {
		free(ptr_gcal->url);
		ptr_gcal->url = NULL;
	}

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


static char *mount_query_url(struct gcal_resource *ptr_gcal,
			     const char *parameters, ...)
{
	va_list ap;
	char *result = NULL, *query_param = NULL, *ptr_tmp = NULL;
	int length;
	char query_separator[] = "&";

	/* TODO: put the google service type string in an array. */
	if (!(strcmp(ptr_gcal->service, "cl")))
		length = sizeof(GCAL_EVENT_START) + sizeof(GCAL_EVENT_END) +
			strlen(ptr_gcal->max_results) + strlen(ptr_gcal->user)
			+ 1;
	else if (!(strcmp(ptr_gcal->service, "cp")))
		length = sizeof(GCONTACT_START) + sizeof(GCONTACT_END) +
			strlen(ptr_gcal->max_results) + strlen(ptr_gcal->user)
			+ 1;
	else
		goto exit;

	result = (char *)malloc(length);
	if (!result)
		goto exit;

	/* This is a basic query URL: must have the google service address
	 * plus the number of max-results returned.
	 */
	if (!(strcmp(ptr_gcal->service, "cl")))
		snprintf(result, length - 1, "%s%s%s%s", GCAL_EVENT_START,
			 ptr_gcal->user, GCAL_EVENT_END, ptr_gcal->max_results);
	else if (!(strcmp(ptr_gcal->service, "cp")))
		snprintf(result, length - 1, "%s%s%s%s", GCONTACT_START,
			 ptr_gcal->user, GCONTACT_END, ptr_gcal->max_results);


	/* For extra query parameters, add "&param_1&param_2&...&param_n" */
	if (parameters) {
		length += strlen(parameters) + sizeof(query_separator);
		ptr_tmp = realloc(result, length);
		if (!ptr_tmp)
			goto cleanup;
		result = ptr_tmp;
		strncat(result, query_separator, sizeof(query_separator));
		strncat(result, parameters, strlen(parameters));

		va_start(ap, parameters);
		while ((query_param = va_arg(ap, char *))) {
			length += strlen(query_param) + sizeof(query_separator);
			ptr_tmp = realloc(result, length);
			if (!ptr_tmp)
				goto cleanup;
			result = ptr_tmp;

			strncat(result, query_separator,
				sizeof(query_separator));
			strncat(result, query_param, strlen(query_param));
		}

	}

	goto exit;

cleanup:
	if (result)
		free(result);
	result = NULL;

exit:
	va_end(ap);
	return result;
}

int gcal_dump(struct gcal_resource *ptr_gcal)
{
	int result = -1;
	char *buffer = NULL;

	if (!ptr_gcal)
		goto exit;
	/* Failed to get authentication token */
	if (!ptr_gcal->auth)
		goto exit;

	buffer = mount_query_url(ptr_gcal, NULL);
	if (!buffer)
		goto exit;

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

int gcal_entry_number(struct gcal_resource *ptr_gcal)
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

struct gcal_event *gcal_get_entries(struct gcal_resource *ptr_gcal,
				    size_t *length)
{

	int result = -1;
	struct gcal_event *ptr_res = NULL;

	if (!ptr_gcal)
		goto exit;

	if (!ptr_gcal->buffer || !ptr_gcal->has_xml)
		goto exit;

	/* create a doc and free atom xml buffer
	 */
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto exit;

	result = get_entries_number(ptr_gcal->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_event) * result);
	if (!ptr_res)
		goto cleanup;

	*length = result;
	result = extract_all_entries(ptr_gcal->document, ptr_res, result);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

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

void gcal_init_event(struct gcal_event *entry)
{
	if (!entry)
		return;

	entry->title = entry->id = entry->edit_uri = NULL;
	entry->content = entry->dt_recurrent = entry->dt_start = NULL;
	entry->dt_end = entry->where = entry->status = NULL;
	entry->updated = NULL;

}

void gcal_destroy_entry(struct gcal_event *entry)
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

void gcal_destroy_entries(struct gcal_event *entries, size_t length)
{
	size_t i = 0;
	if (!entries)
		return;

	for (; i < length; ++i)
		gcal_destroy_entry((entries + i));

	free(entries);
}

/* This function makes possible to share code between 'add'
 * and 'edit' events.
 */
int up_entry(char *data2post, struct gcal_resource *ptr_gcal,
	     const char *url_server, HTTP_CMD up_mode, int expected_code)
{
	int result = -1;
	int length = 0;
	char *h_auth = NULL, *h_length = NULL, *tmp;
	const char header[] = "Content-length: ";
	int (*up_callback)(struct gcal_resource *, const char *,
			   char *, char *, char *,
			   char *, const int);

	if (!data2post || !ptr_gcal)
		goto exit;

	if (!ptr_gcal->auth)
		goto exit;

	if (up_mode == POST)
		up_callback = http_post;
	else if (up_mode == PUT)
		up_callback = http_put;
	else
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(ptr_gcal);

	/* Mounts content length and  authentication header strings */
	length = strlen(data2post) + strlen(header) + 1;
	h_length = (char *) malloc(length) ;
	if (!h_length)
		goto exit;
	strncpy(h_length, header, sizeof(header));
	tmp = h_length + sizeof(header) - 1;
	snprintf(tmp, length - (sizeof(header) + 1), "%d", strlen(data2post));


	length = strlen(ptr_gcal->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, ptr_gcal->auth);


	/* Post the data */
	if (!(strcmp(ptr_gcal->service, "cp"))) {
		/* For contacts, there is *not* redirection. */
		result = up_callback(ptr_gcal, url_server,
				     "Content-Type: application/atom+xml",
				     h_length,
				     h_auth,
				     data2post, expected_code);
		if (!result) {

			result = 0;
			goto cleanup;
		}
	} else if (!(strcmp(ptr_gcal->service, "cl"))) {
		/* For calendar, it *must* be redirection */
		result = up_callback(ptr_gcal, url_server,
				     "Content-Type: application/atom+xml",
				     h_length,
				     h_auth,
				     data2post, GCAL_REDIRECT_ANSWER);
		if (result == -1)
			goto cleanup;
	} else
		goto cleanup;


	if (ptr_gcal->url) {
		free(ptr_gcal->url);
		ptr_gcal->url = NULL;
	}

	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url))
		goto cleanup;

	clean_buffer(ptr_gcal);

	/* Add gsessionid to post URL */
	result = up_callback(ptr_gcal, ptr_gcal->url,
			     "Content-Type: application/atom+xml",
			     h_length,
			     h_auth,
			     data2post, expected_code);

	if (result == -1) {
		if (ptr_gcal->fout_log) {
			fprintf(ptr_gcal->fout_log,
				"result = %s\n", ptr_gcal->buffer);
			fprintf(ptr_gcal->fout_log,
				"\nurl = %s\nh_length = %s\nh_auth = %s"
				"\ndata2post =%s%d\n",
				ptr_gcal->url, h_length, h_auth, data2post,
				strlen(data2post));
		}
		goto cleanup;
	}

cleanup:

	if (h_length)
		free(h_length);
	if (h_auth)
		free(h_auth);

exit:
	return result;
}

int gcal_create_event(struct gcal_resource *ptr_gcal,
		      struct gcal_event *entries,
		      struct gcal_event *updated)
{
	int result = -1, length;
	char *xml_entry = NULL;

	if ((!entries) || (!ptr_gcal))
		return result;

	result = xmlentry_create(entries, &xml_entry, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_entry, ptr_gcal, GCAL_EDIT_URL, POST,
			  GCAL_EDIT_ANSWER);

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_entries(ptr_gcal->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;

xmlclean:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

cleanup:
	if (xml_entry)
		free(xml_entry);

exit:
	return result;
}

int gcal_delete_event(struct gcal_resource *ptr_gcal,
		      struct gcal_event *entry)
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

	curl_easy_setopt(ptr_gcal->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	result = http_post(ptr_gcal, entry->edit_uri,
			   "Content-Type: application/atom+xml",
			   NULL,
			   h_auth,
			   NULL, GCAL_REDIRECT_ANSWER);

	/* Get the gsessionid redirect URL */
	if (result == -1)
		goto cleanup;

	if (ptr_gcal->url) {
		free(ptr_gcal->url);
		ptr_gcal->url = NULL;
	}
	if (get_the_url(ptr_gcal->buffer, ptr_gcal->length, &ptr_gcal->url))
		goto cleanup;

	result = http_post(ptr_gcal, ptr_gcal->url,
			   "Content-Type: application/atom+xml",
			   NULL,
			   h_auth,
			   NULL, GCAL_DEFAULT_ANSWER);

cleanup:

	if (h_auth)
		free(h_auth);

exit:

	return result;

}

int gcal_edit_event(struct gcal_resource *ptr_gcal,
		    struct gcal_event *entry,
		    struct gcal_event *updated)
{

	int result = -1, length;
	char *xml_entry = NULL;

	if ((!entry) || (!ptr_gcal))
		goto exit;

	result = xmlentry_create(entry, &xml_entry, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_entry, ptr_gcal, entry->edit_uri, PUT,
			  GCAL_DEFAULT_ANSWER);

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_entries(ptr_gcal->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;

xmlclean:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

cleanup:
	if (xml_entry)
		free(xml_entry);

exit:
	return result;
}

char *gcal_access_buffer(struct gcal_resource *ptr_gcal)
{
	char *result = NULL;
	if (ptr_gcal)
		if (ptr_gcal->buffer)
			result = ptr_gcal->buffer;

	return result;

}


int get_mili_timestamp(char *timestamp, size_t length, char *atimezone)
{
	struct tm *loctime;
	time_t curtime;
	struct timeval detail_time;
	char buffer[12];

	if (!timestamp || length < TIMESTAMP_SIZE)
		return -1;

	curtime = time(NULL);
	loctime = localtime(&curtime);
	gettimeofday(&detail_time, NULL);

	strftime(timestamp, length - 1, "%FT%T", loctime);
	snprintf(buffer, sizeof(buffer) - 1, ".%03d",
		 (int)detail_time.tv_usec/1000);

	strncat(timestamp, buffer, length);
	if (atimezone)
		strncat(timestamp, atimezone, length);
	else
		strncat(timestamp, "Z", length);


	return 0;
}


/* TODO: move most of this code to a generic 'query' function, since
 * quering for updated entries is just a query with a set of
 * parameters.
 */
int gcal_query_updated(struct gcal_resource *ptr_gcal, char *timestamp)
{
	int result = -1;
	char *query_url = NULL;
	char *query_timestamp = NULL;
	char query_updated_param[] = "updated-min=";
	char query_zone_param[] = "ctz=";
	char *buffer1 = NULL, *buffer2 = NULL, *buffer3 = NULL;
	char *ptr, *hour_const = NULL;
	size_t length = 0;

	if (!ptr_gcal)
		goto exit;

	length = TIMESTAMP_MAX_SIZE + sizeof(query_updated_param) + 1;
	buffer1 = (char *) malloc(length);
	if (!buffer1)
		goto exit;

	if (!timestamp) {
		query_timestamp = (char *)malloc(TIMESTAMP_MAX_SIZE);
		if (!query_timestamp)
			goto cleanup;
		result = get_mili_timestamp(query_timestamp, TIMESTAMP_MAX_SIZE,
					    ptr_gcal->timezone);
		if (result)
			goto cleanup;

		/* Change the hour to 06:00AM plus the timezone when
		 * available.
		 */
		ptr = query_timestamp + strlen(query_timestamp);
		if (ptr_gcal->timezone) {
			hour_const = "06:00:00.000";
			ptr -= strlen(hour_const) + strlen(ptr_gcal->timezone);
		}
		else {
			hour_const = "06:00:00.000Z";
			ptr -= strlen(hour_const);
		}

		while (*hour_const)
			*ptr++ = *hour_const++;

	} else if (timestamp) {
		query_timestamp = strdup(timestamp);
		if (!query_timestamp)
			goto cleanup;
	}

	strcpy(buffer1, query_updated_param);
	strncat(buffer1, query_timestamp, strlen(query_timestamp));

	/* 'showdeleted' is only valid for google contacts */
	if ((ptr_gcal->deleted == SHOW) &&
	    (!(strcmp(ptr_gcal->service, "cp")))) {
		ptr = strdup("showdeleted=true");
		if (!ptr)
			goto cleanup;

		/* Set the query string to the available buffer parameter */
		if (!buffer2)
			buffer2 = ptr;
		else if (!buffer3)
			buffer3 = ptr;
	}

	/* Add location to query (if set) */
	if (ptr_gcal->location) {
		length = strlen(ptr_gcal->location) +
			sizeof(query_zone_param) + 1;
		ptr = (char *) malloc(length);
		if (!ptr)
			goto cleanup;

		strcpy(ptr, query_zone_param);
		strcat(ptr, ptr_gcal->location);

		/* Set the query string to the available buffer parameter */
		if (!buffer2)
			buffer2 = ptr;
		else if (!buffer3)
			buffer3 = ptr;

	}

	/* TODO: implement URL encoding i.e. RFC1738 using
	 * 'curl_easy_escape'.
	 */
	query_url = mount_query_url(ptr_gcal, buffer1, buffer2, buffer3, NULL);
	if (!query_url)
		goto cleanup;

	result = get_follow_redirection(ptr_gcal, query_url);
	if (!result)
		ptr_gcal->has_xml = 1;

cleanup:

	if (query_timestamp)
		free(query_timestamp);
	if (buffer1)
		free(buffer1);
	if (buffer2)
		free(buffer2);
	if (buffer3)
		free(buffer3);
	if (query_url)
		free(query_url);

exit:
	return result;

}

int gcal_set_timezone(struct gcal_resource *ptr_gcal, char *atimezone)
{
	int result = -1;
	if ((!ptr_gcal) || (!atimezone))
		goto exit;

	if (ptr_gcal->timezone)
		free(ptr_gcal->timezone);

	ptr_gcal->timezone = strdup(atimezone);
	if (ptr_gcal->timezone)
		result = 0;

exit:
	return result;
}

int gcal_set_location(struct gcal_resource *ptr_gcal, char *location)
{
	int result = -1;
	if ((!ptr_gcal) || (!location))
		goto exit;

	if (ptr_gcal->location)
		free(ptr_gcal->location);

	ptr_gcal->location = strdup(location);
	if (ptr_gcal->location)
		result = 0;

exit:
	return result;

}

void gcal_deleted(struct gcal_resource *ptr_gcal, display_deleted_entries opt)
{
	if (!ptr_gcal)
		return;

	if (opt == SHOW)
		ptr_gcal->deleted = SHOW;
	else if (opt == HIDE)
		ptr_gcal->deleted = HIDE;
	else if (ptr_gcal->fout_log)
		fprintf(ptr_gcal->fout_log, "gcal_deleted: invalid option:%d\n",
			opt);

}

int gcal_query(struct gcal_resource *ptr_gcal, const char *parameters, ...)
{
	(void)ptr_gcal;
	(void)parameters;

	return -1;
}
