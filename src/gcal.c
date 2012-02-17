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
 * - retrieve a single event
 * - soft/hard edits/deletes (use 'If-Match: *' instead of ETag)
 * - unit test for languages with special characters (it works already)
 * - batch operation (add/edit/delete): will require some new public functions
 *
 * Lower priority
 * - allow user to subscribe to another person calendar
 * - enable user create/edit/delete calendars
 * - think a way to securely store passwords
 * - more utests
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <curl/curl.h>

#include "internal_gcal.h"
#include "gcal.h"
#include "gcal_parser.h"
#include "msvc_hacks.h"
#include "gcontact.h"

#ifdef GCAL_DEBUG_CURL
#include "curl_debug_gcal.h"
#endif

static void reset_buffer(struct gcal_resource *ptr)
{
	if (ptr->buffer)
		free(ptr->buffer);
	ptr->length = 256;
	ptr->buffer = (char *) calloc(ptr->length, sizeof(char));
	ptr->previous_length = 0;
}

struct gcal_resource *gcal_construct(gservice mode)
{
	struct gcal_resource *ptr;
	ptr = malloc(sizeof(struct gcal_resource));
	if (!ptr)
		goto exit;

	ptr->has_xml = 0;
	ptr->document = NULL;
	ptr->user = NULL;
	ptr->domain = NULL;
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
	ptr->store_xml_entry = 0;

	if (!(ptr->buffer) || (!(ptr->curl)) || (!ptr->max_results)) {
		if (ptr->max_results)
			free(ptr->max_results);
		gcal_destroy(ptr);
		ptr = NULL;
		goto exit;
	}

	/* Initializes to google calendar as default */
	if (gcal_set_service(ptr, mode)) {
		free(ptr);
		ptr = NULL;
	}

exit:
	return ptr;
}

int gcal_set_service(struct gcal_resource *gcalobj, gservice mode)
{
	int result = 0;

	if (gcalobj) {
		if (mode == GCALENDAR)
			strcpy(gcalobj->service, "cl");
		else if (mode == GCONTACT)
			strcpy(gcalobj->service, "cp");
		else
			result = -1;

	}

	return result;

}

void clean_buffer(struct gcal_resource *gcal_obj)
{
	if (gcal_obj) {
		memset(gcal_obj->buffer, 0, gcal_obj->length);
		gcal_obj->previous_length = 0;
	}
}

static void _gcal_destroy(struct gcal_resource *gcal_obj, int free_obj)
{
	if (!gcal_obj)
		return;

	if (gcal_obj->buffer)
		free(gcal_obj->buffer);
	if (gcal_obj->curl && free_obj == 0)
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
	if (gcal_obj->fout_log && free_obj == 0)
		fclose(gcal_obj->fout_log);
	if (gcal_obj->max_results)
		free(gcal_obj->max_results);
	if (gcal_obj->timezone)
		free(gcal_obj->timezone);
	if (gcal_obj->location)
		free(gcal_obj->location);
	if (gcal_obj->domain)
		free(gcal_obj->domain);

	if (free_obj == 0) {
		free(gcal_obj);
	}
}

void gcal_destroy(struct gcal_resource *gcal_obj)
{
	_gcal_destroy(gcal_obj, 0);
}

static size_t write_cb(void *ptr, size_t count, size_t chunk_size, void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *)data;
	int current_length = strlen(gcal_ptr->buffer);
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

static int check_request_error(struct gcal_resource *gcalobj, int code,
			       int expected_answer)
{
	int result = 0;
	CURL *curl_ctx = gcalobj->curl;

	curl_easy_getinfo(curl_ctx, CURLINFO_HTTP_CODE,
			  &(gcalobj->http_code));
	if (code || (gcalobj->http_code != expected_answer)) {

		if (gcalobj->curl_msg)
			free(gcalobj->curl_msg);

		gcalobj->curl_msg = strdup(curl_easy_strerror(code));

		if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "%s\n%s%s\n%s%d\n",
				"check_request_error: failed request.",
				"Curl code: ", gcalobj->curl_msg,
				"HTTP code: ", (int)gcalobj->http_code);
		result = -1;
	}

	return result;
}

static int common_upload(struct gcal_resource *gcalobj,
			 char *header, char *header2, char *header3,
			 char *header4,
			 struct curl_slist **curl_headers,
			 const char *gdata_version)
{
	int result = -1;
	CURL *curl_ctx = gcalobj->curl;
	struct curl_slist *response_headers = NULL;

#ifdef GCAL_DEBUG_CURL
	struct data_curl_debug flag;
	flag.trace_ascii = 1;
	curl_easy_setopt(gcalobj->curl, CURLOPT_DEBUGFUNCTION,
			 curl_debug_gcal_trace);
	curl_easy_setopt(gcalobj->curl, CURLOPT_DEBUGDATA, &flag);
	curl_easy_setopt(gcalobj->curl, CURLOPT_VERBOSE, 1);
#endif

	/* To support Google Data API 2.0 */
	response_headers = curl_slist_append(response_headers,
					     gdata_version);

	if (header)
		response_headers = curl_slist_append(response_headers, header);
	if (header2)
		response_headers = curl_slist_append(response_headers, header2);
	if (header3)
		response_headers = curl_slist_append(response_headers, header3);
	if (header4)
		response_headers = curl_slist_append(response_headers, header4);

	if (!response_headers)
		return result;

	*curl_headers = response_headers;

	curl_easy_setopt(curl_ctx, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, (void *)gcalobj);

	return result = 0;
}

int http_post(struct gcal_resource *gcalobj, const char *url,
	      char *header, char *header2, char *header3,
	      char *header4,
	      char *post_data, unsigned int length,
	      const int expected_answer,
	      const char *gdata_version)
{
	int result = -1;
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	CURL *curl_ctx;
	if (!gcalobj)
		goto exit;

	curl_ctx = gcalobj->curl;
	result = common_upload(gcalobj, header, header2, header3, header4,
			       &response_headers,
			       gdata_version);
	if (result)
		goto exit;

	/* It seems deprecated, as long I set POSTFIELDS */
	curl_easy_setopt(curl_ctx, CURLOPT_POST, 1);
	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	if (post_data) {
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE,
				 length);
	}
	else
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, 0);

	res = curl_easy_perform(curl_ctx);
	result = check_request_error(gcalobj, res, expected_answer);

	/* cleanup */
	curl_slist_free_all(response_headers);

exit:
	return result;

}

static int http_put(struct gcal_resource *gcalobj, const char *url,
		    char *header, char *header2, char *header3,
		    char *header4,
		    char *post_data, unsigned int length,
		    const int expected_answer,
		    const char *gdata_version)
{
	int result = -1;
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	CURL *curl_ctx;
	if (!gcalobj)
		goto exit;

	curl_ctx = gcalobj->curl;
	result = common_upload(gcalobj, header, header2, header3, header4,
			       &response_headers,
			       gdata_version);
	if (result)
		goto exit;

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	/* Tells curl that I want to PUT */
	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, "PUT");

	if (post_data) {
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE,
				length);
	}
	else
		curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, 0);



	res = curl_easy_perform(curl_ctx);
	result = check_request_error(gcalobj, res, expected_answer);

	/* cleanup */
	curl_slist_free_all(response_headers);

	/* Restores curl context to previous standard mode */
	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, NULL);

exit:
	return result;

}

int gcal_get_authentication(struct gcal_resource *gcalobj,
			    char *user, char *password)
{

	int post_len = 0;
	char *post = NULL;
	int result = -1;
	char *tmp = NULL;
	char *buffer = NULL;
	char *enc_user = NULL;
	char *enc_password = NULL;

	if (!gcalobj || !user || !password)
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(gcalobj);

	/* Properly encode user and password */
	enc_user = curl_easy_escape(gcalobj->curl, user, strlen(user));
	enc_password = curl_easy_escape(gcalobj->curl, password,
					strlen(password));
	if ((!enc_password) || (!enc_user))
		goto cleanup;

	post_len = strlen(enc_user) + strlen(enc_password) +
		   sizeof(ACCOUNT_TYPE) +
		   sizeof(EMAIL_FIELD) +
		   sizeof(PASSWD_FIELD) + sizeof(SERVICE_FIELD) +
		   strlen(gcalobj->service) + sizeof(CLIENT_SOURCE)
		   + 5; /* thanks to 4 '&' between fields + null character */
	post = (char *) malloc(post_len);
	if (!post)
		goto cleanup;

	snprintf(post, post_len - 1,
		 "%s&"
		 "%s%s&"
		 "%s%s&"
		 "%s%s&"
		 "%s",
		 ACCOUNT_TYPE,
		 EMAIL_FIELD, enc_user,
		 PASSWD_FIELD, enc_password,
		 SERVICE_FIELD, gcalobj->service,
		 CLIENT_SOURCE);

	result = http_post(gcalobj, GCAL_URL,
			   "Content-Type: application/x-www-form-urlencoded",
			   NULL, NULL, NULL, post, strlen(post),
			   GCAL_DEFAULT_ANSWER,
			   "GData-Version: 2");

	if ((tmp = strstr(user, "@"))) {
		if (!(buffer = strdup(user)))
			goto cleanup;

		buffer[tmp - user] = '\0';
		if (!(gcalobj->user = strdup(buffer)))
			goto cleanup;

		++tmp;
		if (!(gcalobj->domain = strdup(tmp)))
			goto cleanup;

		free(buffer);
	} else {
		gcalobj->user = strdup(user);
		gcalobj->domain = strdup("gmail.com");
	}

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
	if (gcalobj->auth)
		free(gcalobj->auth);

	gcalobj->auth = strstr(gcalobj->buffer, HEADER_AUTH);
	gcalobj->auth = strdup(gcalobj->auth + strlen(HEADER_AUTH));
	if (!gcalobj->auth)
		goto cleanup;

	tmp = strstr(gcalobj->auth, "\n");
	if (tmp)
		*tmp = '\0';

	result = 0;

cleanup:
	if (enc_user)
	    curl_free(enc_user);
	if (enc_password)
	    curl_free(enc_password);
	if (post)
		free(post);

exit:
	return result;

}

int get_follow_redirection(struct gcal_resource *gcalobj, const char *url,
			   void *cb_download, const char *gdata_version)
{
	struct curl_slist *response_headers = NULL;
	int length = 0;
	int result = -1;
	char *tmp_buffer = NULL;
	void *downloader = NULL;

	if (cb_download == NULL)
		downloader = write_cb;
	else
		downloader = cb_download;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(gcalobj);

	if (!gcalobj->auth)
		goto exit;
	length = strlen(gcalobj->auth) + sizeof(HEADER_GET) + 1;
	tmp_buffer = (char *) malloc(length);
	if (!tmp_buffer)
		goto exit;
	snprintf(tmp_buffer, length - 1, "%s%s", HEADER_GET, gcalobj->auth);

	/* To support Google Data API 2.0 */
	response_headers = curl_slist_append(response_headers,
					     gdata_version);

	response_headers = curl_slist_append(response_headers, tmp_buffer);
	if (!response_headers)
		return result;

	curl_easy_setopt(gcalobj->curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(gcalobj->curl, CURLOPT_HTTPHEADER, response_headers);
	curl_easy_setopt(gcalobj->curl, CURLOPT_URL, url);
	curl_easy_setopt(gcalobj->curl, CURLOPT_WRITEFUNCTION, downloader);
	curl_easy_setopt(gcalobj->curl, CURLOPT_WRITEDATA, (void *)gcalobj);

	result = curl_easy_perform(gcalobj->curl);

	if (!(strcmp(gcalobj->service, "cp"))) {
		/* For contacts, there is *not* redirection. */
		if (!(result = check_request_error(gcalobj, result,
						   GCAL_DEFAULT_ANSWER))) {
			result = 0;
			goto cleanup;
		}
	} else if (!(strcmp(gcalobj->service, "cl"))) {
		/* For calendar, it *must* be redirection */
		if (check_request_error(gcalobj, result,
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
	if (gcalobj->url) {
		free(gcalobj->url);
		gcalobj->url = NULL;
	}

	if (get_the_url(gcalobj->buffer, gcalobj->length, &gcalobj->url)) {
		result = -1;
		goto cleanup;
	}

	clean_buffer(gcalobj);
	curl_easy_setopt(gcalobj->curl, CURLOPT_URL, gcalobj->url);
	result = curl_easy_perform(gcalobj->curl);
	if ((result = check_request_error(gcalobj, result,
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


static char *mount_query_url(struct gcal_resource *gcalobj,
			     const char *parameters, ...)
{
	va_list ap;
	char *result = NULL, *query_param = NULL, *ptr_tmp = NULL;
	int length;
	char query_separator[] = "&";
	char query_init[] = "?";
	/* By default, google contacts are not ordered */
	char contact_order[] = "&orderby=lastmodified";
	if (!gcalobj)
		goto exit;

	if ((!gcalobj->user))
		goto exit;

	/* TODO: put the google service type string in an array. */
	if (!(strcmp(gcalobj->service, "cl"))) {
		if (gcalobj->max_results)
			length = sizeof(GCAL_EVENT_START) +
				sizeof(GCAL_DELIMITER) +
				strlen(gcalobj->domain) +
				sizeof(GCAL_EVENT_END) +
				sizeof(query_init) +
				strlen(gcalobj->max_results) +
				strlen(gcalobj->user) + 1;
		else
			length = sizeof(GCAL_EVENT_START) +
				sizeof(GCAL_DELIMITER) +
				strlen(gcalobj->domain) +
				sizeof(GCAL_EVENT_END) +
				sizeof(query_init) +
				strlen(gcalobj->user) + 1;

	}
	else if (!(strcmp(gcalobj->service, "cp"))) {
		if (gcalobj->max_results)
			length = sizeof(GCONTACT_START) +
				sizeof(GCAL_DELIMITER) +
				strlen(gcalobj->domain) +
				sizeof(GCONTACT_END) +
				sizeof(query_init) +
				strlen(gcalobj->max_results) +
				strlen(gcalobj->user) +
				sizeof(contact_order) + 1;
		else
			length = sizeof(GCONTACT_START) +
				sizeof(GCAL_DELIMITER) +
				strlen(gcalobj->domain) +
				sizeof(GCONTACT_END) +
				sizeof(query_init) +
				strlen(gcalobj->user) + 1;

	} else
		goto exit;

	result = (char *)malloc(length);
	if (!result)
		goto exit;

	if (!(strcmp(gcalobj->service, "cl"))) {
		/* This is a basic query URL: must have the google service
		 * address plus the number of max-results returned.
		 */
		if (gcalobj->max_results)
			snprintf(result, length - 1, "%s%s%s%s%s%s%s",
				 GCAL_EVENT_START, gcalobj->user,
				 GCAL_DELIMITER, gcalobj->domain,
				 GCAL_EVENT_END, query_init,
				 gcalobj->max_results);
		else
			snprintf(result, length - 1, "%s%s%s%s%s%s",
				 GCAL_EVENT_START, gcalobj->user,
				 GCAL_DELIMITER, gcalobj->domain,
				 GCAL_EVENT_END, query_init);

	} else if (!(strcmp(gcalobj->service, "cp"))) {
		if (gcalobj->max_results)
			snprintf(result, length - 1, "%s%s%s%s%s%s%s%s",
				 GCONTACT_START, gcalobj->user,
				 GCAL_DELIMITER, gcalobj->domain,
				 GCONTACT_END, query_init,
				 gcalobj->max_results,
				 contact_order);
		else
			snprintf(result, length - 1, "%s%s%s%s%s%s",
				 GCONTACT_START, gcalobj->user,
				 GCAL_DELIMITER, gcalobj->domain,
				 GCONTACT_END, query_init);
	}

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

int gcal_dump(struct gcal_resource *gcalobj, const char *gdata_version)
{
	int result = -1;
	char *buffer = NULL;

	if (!gcalobj)
		goto exit;
	/* Failed to get authentication token */
	if (!gcalobj->auth)
		goto exit;

	buffer = mount_query_url(gcalobj, NULL);
	if (!buffer)
		goto exit;

	result =  get_follow_redirection(gcalobj, buffer, NULL, gdata_version);

	if (!result)
		gcalobj->has_xml = 1;

	free(buffer);
exit:

	return result;
}

void gcal_cleanup_calendar(struct gcal_resource_array *resource_array)
{
	size_t		i;

	if (!resource_array)
		return;

	for(i = 0; i < resource_array->length; i++) {
		_gcal_destroy(&(resource_array->entries[i]), 1);
	}

	free(resource_array->entries);

	resource_array->length = 0;
	resource_array->entries = NULL;
}

int gcal_get_calendar_by_index(struct gcal_resource_array *gcal_array, size_t _index,
			       gcal_t *gcalobj)
{
	int	result = -1;

	if (!gcal_array || !gcalobj)
		goto exit;

	if (_index > gcal_array->length)
		goto exit;

	*gcalobj = &gcal_array->entries[_index];
	result = 0;

exit:
	return result;
}

int gcal_get_calendar(struct gcal_resource_array *gcal_array,
		      const char *user, const char *domain,
		      gcal_t *gcalobj)
{
	size_t		i;

	if (!gcal_array || !user || !domain || !gcalobj)
		goto exit;

	for (i = 0; i < gcal_array->length; i++) {
		if (gcal_array->entries[i].user && gcal_array->entries[i].domain &&
		    !strncmp(gcal_array->entries[i].user, user,
			     strlen(gcal_array->entries[i].user)) &&
		    !strncmp(gcal_array->entries[i].domain, domain,
			     strlen(gcal_array->entries[i].domain))) {
			*gcalobj = &gcal_array->entries[i];
			return 0;
		}
	}

exit:
	return -1;
}

int gcal_calendar_list(struct gcal_resource *gcalobj,
		       struct gcal_resource_array *gcal_array)
{
	int result = 0;
	size_t i;

	if (gcal_array)
		gcal_array->length = 0;

	if ((!gcalobj) || (!gcal_array))
		goto exit;

	result =  get_follow_redirection(gcalobj, GCAL_LIST, NULL,
			"GData-Version: 2");
	if (!result) {
		gcalobj->has_xml = 1;
	}

	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto exit;

	result = get_entries_number_xml(gcalobj->document);
	if (result == -1)
		goto exit;

	gcal_array->length = result;
	result = -1;
	gcal_array->entries = malloc(sizeof(struct gcal_resource) * gcal_array->length);
	if (!gcal_array->entries) {
		goto cleanup;
	}
	memset(gcal_array->entries, 0, sizeof(struct gcal_resource) * gcal_array->length);

	for (i = 0; i < gcal_array->length; i++) {
		gcal_array->entries[i].has_xml = 1;
		gcal_array->entries[i].curl = gcalobj->curl;
		gcal_array->entries[i].auth = strdup(gcalobj->auth);
		gcal_array->entries[i].buffer = NULL;
		gcal_array->entries[i].document = NULL;
		reset_buffer(&gcal_array->entries[i]);
		gcal_array->entries[i].max_results = strdup(GCAL_UPPER);
		gcal_set_service(&(gcal_array->entries[i]), GCALENDAR);

		result = get_calendar_entry(gcalobj->document, i, &gcal_array->entries[i]);
		if (result == -1) {
			free(gcal_array->entries);

			gcal_array->length = 0;
			gcal_array->entries = NULL;

			goto exit;
		}
	}

cleanup:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

exit:
	if (gcalobj->url) {
		free(gcalobj->url);
		gcalobj->url = NULL;
	}

	return result;
}

int gcal_entry_number(struct gcal_resource *gcalobj)
{
	int result = -1;

	if (!gcalobj)
		goto exit;
	/* Failed to get authentication token */
	if (!gcalobj->auth)
		goto exit;

	if (!gcalobj->buffer || !gcalobj->has_xml)
		goto exit;

	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto exit;

	result = get_entries_number(gcalobj->document);
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

exit:
	return result;
}

struct gcal_event *gcal_get_entries(struct gcal_resource *gcalobj,
				    size_t *length)
{

	int result = -1, i;
	struct gcal_event *ptr_res = NULL;

	if (!gcalobj)
		goto exit;

	if (!gcalobj->buffer || !gcalobj->has_xml)
		goto exit;

	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto exit;

	result = get_entries_number(gcalobj->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_event) * result);
	if (!ptr_res) {
		*length = 0;
		goto cleanup;
	}
	memset(ptr_res, 0, sizeof(struct gcal_event) * result);

	*length = result;

	for (i = 0; i < result; ++i) {
		gcal_init_event((ptr_res + i));
		if (gcalobj->store_xml_entry)
			(ptr_res + i)->common.store_xml = 1;
	}

	result = extract_all_entries(gcalobj->document, ptr_res, result);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

cleanup:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

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

	entry->common.store_xml = entry->common.deleted = 0;
	entry->common.title = entry->common.id = NULL;
	entry->common.edit_uri = entry->common.etag = NULL;
	entry->common.xml = entry->common.updated = NULL;
	entry->common.published = NULL;
	entry->content = entry->dt_recurrent = entry->dt_start = NULL;
	entry->dt_end = entry->where = entry->status = NULL;
	entry->anyoneCanAddSelf = entry->guestsCanInviteOthers = NULL;
	entry->guestsCanModify = entry->guestsCanSeeGuests = NULL;
	entry->sequence = entry->common.visibility = NULL;
	entry->attendees = NULL;
	entry->alarms = NULL;
	entry->alarms_nr = 0;
	entry->attendees_nr = 0;
}

void gcal_destroy_entry(struct gcal_event *entry)
{
	if (!entry)
		return;

	clean_string(entry->common.title);
	clean_string(entry->common.id);
	clean_string(entry->common.edit_uri);
	clean_string(entry->common.etag);
	clean_string(entry->common.updated);
	clean_string(entry->common.published);
	clean_string(entry->common.visibility);
	clean_string(entry->common.xml);
	clean_string(entry->content);
	clean_string(entry->dt_recurrent);
	clean_string(entry->dt_start);
	clean_string(entry->dt_end);
	clean_string(entry->where);
	clean_string(entry->status);
	clean_string(entry->anyoneCanAddSelf);
	clean_string(entry->guestsCanInviteOthers);
	clean_string(entry->guestsCanModify);
	clean_string(entry->guestsCanSeeGuests);
	clean_string(entry->sequence);
	if(entry->attendees) {
		if(entry->attendees->email) {
			clean_string(entry->attendees->email);
		}
		free(entry->attendees);
	}
	if(entry->alarms) {
		free(entry->alarms);
	}
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
int up_entry(char *data2post, unsigned int m_length,
	     struct gcal_resource *gcalobj,
	     const char *url_server, char *etag,
	     HTTP_CMD up_mode, char *content_type,
	     int expected_code)
{
	int result = -1;
	int length = 0;
	char *h_auth = NULL, *h_length = NULL, *tmp, *content;
	const char header[] = "Content-length: ";
	int (*up_callback)(struct gcal_resource *, const char *,
			   char *, char *, char *, char *,
			   char *, unsigned int, const int,
			   const char *);

	if (!data2post || !gcalobj)
		goto exit;

	if (!gcalobj->auth)
		goto exit;

	if (up_mode == POST)
		up_callback = http_post;
	else if (up_mode == PUT)
		up_callback = http_put;
	else
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(gcalobj);

	/* Mounts content length and  authentication header strings */
	length = m_length + strlen(header) + 1;
	h_length = (char *) malloc(length) ;
	if (!h_length)
		goto exit;
	strncpy(h_length, header, sizeof(header));
	tmp = h_length + sizeof(header) - 1;
	snprintf(tmp, length - (sizeof(header) + 1), "%d", m_length);


	length = strlen(gcalobj->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, gcalobj->auth);


	if (!content_type)
		content = "Content-Type: application/atom+xml";
	else
		content = content_type;

	/* Post the data */
	if (!(strcmp(gcalobj->service, "cp"))) {
		/* For contacts, there is *not* redirection. */
		result = up_callback(gcalobj, url_server,
				     content,
				     h_length,
				     h_auth,
				     etag,
				     data2post, m_length,
				     expected_code,
				     "GData-Version: 3.0");
		if (!result) {

			result = 0;
			goto cleanup;
		}
	} else if (!(strcmp(gcalobj->service, "cl"))) {
		/* For calendar, it *must* be redirection */
		result = up_callback(gcalobj, url_server,
				     content,
				     h_length,
				     h_auth,
				     etag,
				     data2post, m_length,
				     GCAL_REDIRECT_ANSWER,
				     "GData-Version: 2");
		if (result == -1) {
			/* XXX: there is one report where google server
			 * doesn't always return redirection.
			 */
			if (gcalobj->http_code == expected_code)
				result = 0;

			goto cleanup;
		}
	} else
		goto cleanup;


	if (gcalobj->url) {
		free(gcalobj->url);
		gcalobj->url = NULL;
	}

	if (get_the_url(gcalobj->buffer, gcalobj->length, &gcalobj->url))
		goto cleanup;

	clean_buffer(gcalobj);

	/* Add gsessionid to post URL */
	if (!(strcmp(gcalobj->service, "cp"))) {
		result = up_callback(gcalobj, gcalobj->url,
				"Content-Type: application/atom+xml",
				h_length,
				h_auth,
				etag,
				data2post, m_length,
				expected_code,
				"GData-Version: 3.0");
	} else if (!(strcmp(gcalobj->service, "cl"))) {
		result = up_callback(gcalobj, gcalobj->url,
				"Content-Type: application/atom+xml",
				h_length,
				h_auth,
				etag,
				data2post, m_length,
				expected_code,
				"GData-Version: 2");
	} else
		goto cleanup;

	if (result == -1) {
		if (gcalobj->fout_log) {
			fprintf(gcalobj->fout_log,
				"result = %s\n", gcalobj->buffer);
			fprintf(gcalobj->fout_log,
				"\nurl = %s\nh_length = %s\nh_auth = %s"
				"\ndata2post =%s%d\n",
				gcalobj->url, h_length, h_auth, data2post,
				m_length);
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

int gcal_create_event(struct gcal_resource *gcalobj,
		      struct gcal_event *entries,
		      struct gcal_event *updated)
{
	int result = -1, length;
	char *xml_entry = NULL;

	if ((!entries) || (!gcalobj))
		return result;

	result = xmlentry_create(entries, &xml_entry, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_entry, strlen(xml_entry),
			  gcalobj, GCAL_EDIT_URL, NULL,
			  POST, NULL, GCAL_EDIT_ANSWER);
	if (result)
		goto cleanup;

	/* Copy raw XML */
	if (gcalobj->store_xml_entry) {
		if (entries->common.xml)
			free(entries->common.xml);
		if (!(entries->common.xml = strdup(gcalobj->buffer)))
			goto cleanup;
	}

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_entries(gcalobj->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;

xmlclean:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

cleanup:
	if (xml_entry)
		free(xml_entry);

exit:
	return result;
}

int gcal_delete_event(struct gcal_resource *gcalobj,
		      struct gcal_event *entry)
{
	int result = -1, length;
	char *h_auth;

	if ((!entry) || (!gcalobj) || (!gcalobj->auth))
		goto exit;

	/* Must cleanup HTTP buffer between requests */
	clean_buffer(gcalobj);

	length = strlen(gcalobj->auth) + sizeof(HEADER_GET) + 1;
	h_auth = (char *) malloc(length);
	if (!h_auth)
		goto exit;
	snprintf(h_auth, length - 1, "%s%s", HEADER_GET, gcalobj->auth);

	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	result = http_post(gcalobj, entry->common.edit_uri,
			   "Content-Type: application/atom+xml",
			   /* Google Data API 2.0 requires ETag */
			   "If-Match: *",
			   h_auth,
			   NULL, NULL, 0, GCAL_REDIRECT_ANSWER,
			   "GData-Version: 2");

	if (result == -1) {
		/* XXX: there is one report where google server
		 * doesn't always return redirection and deletes
		 * the entry right away!
		 */
		if (gcalobj->http_code == GCAL_DEFAULT_ANSWER)
			result = 0;

		goto cleanup;
	}

	/* Get the gsessionid redirect URL */
	if (gcalobj->url) {
		free(gcalobj->url);
		gcalobj->url = NULL;
	}
	if (get_the_url(gcalobj->buffer, gcalobj->length, &gcalobj->url))
		goto cleanup;

	result = http_post(gcalobj, gcalobj->url,
			   "Content-Type: application/atom+xml",
			   /* Google Data API 2.0 requires ETag */
			   "If-Match: *",
			   h_auth,
			   NULL, NULL, 0, GCAL_DEFAULT_ANSWER,
			   "GData-Version: 2");

cleanup:
	/* Restores curl context to previous standard mode */
	curl_easy_setopt(gcalobj->curl, CURLOPT_CUSTOMREQUEST, NULL);

	if (h_auth)
		free(h_auth);

exit:

	return result;

}

int gcal_edit_event(struct gcal_resource *gcalobj,
		    struct gcal_event *entry,
		    struct gcal_event *updated)
{

	int result = -1, length;
	char *xml_entry = NULL;

	if ((!entry) || (!gcalobj))
		goto exit;

	result = xmlentry_create(entry, &xml_entry, &length);
	if (result == -1)
		goto exit;

	result = up_entry(xml_entry, strlen(xml_entry),
			  gcalobj, entry->common.edit_uri,
			  /* Google Data API 2.0 requires ETag */
			  "If-Match: *",
			  PUT, NULL, GCAL_DEFAULT_ANSWER);
	if (result)
		goto cleanup;

	/* Copy raw XML */
	if (gcalobj->store_xml_entry) {
		if (entry->common.xml)
			free(entry->common.xml);
		if (!(entry->common.xml = strdup(gcalobj->buffer)))
			goto cleanup;
	}

	/* Parse buffer and create the new contact object */
	if (!updated)
		goto cleanup;
	result = -2;
	gcalobj->document = build_dom_document(gcalobj->buffer);
	if (!gcalobj->document)
		goto cleanup;

	/* There is only one 'entry' in the buffer */
	result = extract_all_entries(gcalobj->document, updated, 1);
	if (result == -1)
		goto xmlclean;

	result = 0;

xmlclean:
	clean_dom_document(gcalobj->document);
	gcalobj->document = NULL;

cleanup:
	if (xml_entry)
		free(xml_entry);

exit:
	return result;
}

char *gcal_access_buffer(struct gcal_resource *gcalobj)
{
	char *result = NULL;
	if (gcalobj)
		if (gcalobj->buffer)
			result = gcalobj->buffer;

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
int gcal_query_updated(struct gcal_resource *gcalobj, char *timestamp,
		const char *gdata_version)
{
	int result = -1;
	char *query_url = NULL;
	char *query_timestamp = NULL;
	char query_updated_param[] = "updated-min=";
	char query_zone_param[] = "ctz=";
	char *buffer1 = NULL, *buffer2 = NULL, *buffer3 = NULL;
	char *ptr, *hour_const = NULL;
	size_t length = 0;

	if (!gcalobj)
		goto exit;

	/* Failed to get authentication token */
	if (!gcalobj->auth)
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
					    gcalobj->timezone);
		if (result)
			goto cleanup;

		result = -1;

		/* Change the hour to 06:00AM plus the timezone when
		 * available.
		 */
		ptr = query_timestamp + strlen(query_timestamp);
		if (gcalobj->timezone) {
			hour_const = "06:00:00.000";
			ptr -= strlen(hour_const) + strlen(gcalobj->timezone);
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
	if ((gcalobj->deleted == SHOW) &&
	    (!(strcmp(gcalobj->service, "cp")))) {
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
	if (gcalobj->location) {
		length = strlen(gcalobj->location) +
			sizeof(query_zone_param) + 1;
		ptr = (char *) malloc(length);
		if (!ptr)
			goto cleanup;

		strcpy(ptr, query_zone_param);
		strcat(ptr, gcalobj->location);

		/* Set the query string to the available buffer parameter */
		if (!buffer2)
			buffer2 = ptr;
		else if (!buffer3)
			buffer3 = ptr;

	}

	/* TODO: implement URL encoding i.e. RFC1738 using
	 * 'curl_easy_escape'.
	 */
	query_url = mount_query_url(gcalobj, buffer1, buffer2, buffer3, NULL);
	if (!query_url)
		goto cleanup;

	result = get_follow_redirection(gcalobj, query_url, NULL, gdata_version);
	if (!result)
		gcalobj->has_xml = 1;

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

int gcal_set_timezone(struct gcal_resource *gcalobj, char *atimezone)
{
	int result = -1;
	if ((!gcalobj) || (!atimezone))
		goto exit;

	if (gcalobj->timezone)
		free(gcalobj->timezone);

	gcalobj->timezone = strdup(atimezone);
	if (gcalobj->timezone)
		result = 0;

exit:
	return result;
}

int gcal_set_location(struct gcal_resource *gcalobj, char *location)
{
	int result = -1;
	if ((!gcalobj) || (!location))
		goto exit;

	if (gcalobj->location)
		free(gcalobj->location);

	gcalobj->location = strdup(location);
	if (gcalobj->location)
		result = 0;

exit:
	return result;

}

void gcal_set_store_xml(struct gcal_resource *gcalobj, char flag)
{
	if ((!gcalobj))
		return;

	gcalobj->store_xml_entry = flag;
}

void gcal_set_proxy(struct gcal_resource *gcalobj, char *proxy)
{
	if ((!gcalobj) || (!proxy)) {
		if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "Invalid proxy!\n");
		return;
	} else
		if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "\n\nproxy: %s\n\n", proxy);

	curl_easy_setopt(gcalobj->curl, CURLOPT_PROXY, proxy);
}

void gcal_set_proxytype(struct gcal_resource *gcalobj, int proxytype) 
{
	if ((!gcalobj) || (!proxytype)) {
		if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "Invalid proxytype!\n");
	} else
		if (gcalobj->fout_log)
			fprintf(gcalobj->fout_log, "\n\nproxytype: %d\n\n", proxytype);

	curl_easy_setopt(gcalobj->curl, CURLOPT_PROXYTYPE, proxytype);
}

void gcal_set_ca_info(struct gcal_resource *gcalobj, char *ca_info)
{
	if (!gcalobj)
		return;

	curl_easy_setopt(gcalobj->curl, CURLOPT_CAINFO, ca_info);
}

void gcal_set_ca_path(struct gcal_resource *gcalobj, char *ca_path)
{
	if (!gcalobj)
		return;

	curl_easy_setopt(gcalobj->curl, CURLOPT_CAPATH, ca_path);
}

void gcal_set_curl_debug_callback(struct gcal_resource *gcalobj, void *debug_callback)
{
	if (!gcalobj)
		return;

	curl_easy_setopt(gcalobj->curl, CURLOPT_DEBUGFUNCTION, debug_callback);
	curl_easy_setopt(gcalobj->curl, CURLOPT_VERBOSE, debug_callback != NULL ? 1 : 0);
}

void gcal_deleted(struct gcal_resource *gcalobj, display_deleted_entries opt)
{
	if (!gcalobj)
		return;

	if (opt == SHOW)
		gcalobj->deleted = SHOW;
	else if (opt == HIDE)
		gcalobj->deleted = HIDE;
	else if (gcalobj->fout_log)
		fprintf(gcalobj->fout_log, "gcal_deleted: invalid option:%d\n",
			opt);

}

int gcal_query(struct gcal_resource *gcalobj, const char *parameters,
		const char *gdata_version)
{
	char *query_url = NULL, *ptr_tmp;
	int result = -1;

	if ((!gcalobj) || (!parameters))
		goto exit;

	/* Swaps the max-results internal member for NULL. This makes
	 * possible a generic query with user defined max-results.
	 */
	ptr_tmp = gcalobj->max_results;
	gcalobj->max_results = NULL;
	query_url = mount_query_url(gcalobj, parameters, NULL);
	gcalobj->max_results = ptr_tmp;
	if (!query_url)
		goto exit;

	result = get_follow_redirection(gcalobj, query_url, NULL,
			gdata_version);

	if (!result)
		gcalobj->has_xml = 1;

	if (query_url)
		free(query_url);
exit:

	return result;
}

char *gcal_get_id(struct gcal_entry *entry)
{
	if (entry)
		return entry->id;

	return NULL;
}

char *gcal_get_xml(struct gcal_entry *entry)
{
	if (entry)
		return entry->xml;

	return NULL;
}

char gcal_get_deleted(struct gcal_entry *entry)
{

	if (entry)
		return entry->deleted;

	return -1;
}

char *gcal_get_published(struct gcal_entry *entry)
{
	if (entry)
		return entry->published;

	return NULL;

}

char *gcal_get_updated(struct gcal_entry *entry)
{
	if (entry)
		return entry->updated;

	return NULL;

}

char *gcal_get_visibility(struct gcal_entry *entry)
{
	if (entry)
		return entry->visibility;

	return NULL;

}

char *gcal_get_title(struct gcal_entry *entry)
{
	if (entry)
		return entry->title;

	return NULL;

}

char *gcal_get_url(struct gcal_entry *entry)
{
	if (entry)
		return entry->edit_uri;

	return NULL;
}

char *gcal_get_etag(struct gcal_entry *entry)
{
	if (entry)
		return entry->etag;

	return NULL;
}

void gcal_final_cleanup()
{
	xmlCleanupParser();
}

char *gcal_resource_get_url(struct gcal_resource *res)
{
	if (res)
		return res->url;

	return NULL;
}

char *gcal_resource_get_user(struct gcal_resource *res)
{
	if (res)
		return res->user;

	return NULL;
}

char *gcal_resource_get_domain(struct gcal_resource *res)
{
	if (res)
		return res->domain;

	return NULL;
}

char *gcal_resource_get_timezone(struct gcal_resource *res)
{
	if (res)
		return res->timezone;

	return NULL;
}

char *gcal_resource_get_location(struct gcal_resource *res)
{
	if (res)
		return res->location;

	return NULL;
}
