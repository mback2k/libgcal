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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "gcal.h"

static const char GCAL_URL[] = "https://www.google.com/accounts/ClientLogin";
static const int GCAL_DEFAULT_ANSWER = 200;
static const char EMAIL_FIELD[] = "Email=";
static const char EMAIL_ADDRESS[] = "@gmail.com";
static const char PASSWD_FIELD[] = "Passwd=";
static const char TRAILING_FIELD[] = "service=cl&source=libgcal";
static const char HEADER_BREAK = '\n';

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
};

struct gcal_resource *gcal_initialize(void)
{

	struct gcal_resource *ptr;
	ptr = (struct gcal_resource *) malloc(sizeof(struct gcal_resource));
	if (!ptr) {
		fprintf(stderr, "%s\n", "gcal_initialize: failed malloc\n");
		goto exit;
	}

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


void gcal_destroy(struct gcal_resource *gcal_obj)
{

	if (gcal_obj->buffer)
		free(gcal_obj->buffer);
	if (gcal_obj->curl)
		curl_easy_cleanup(gcal_obj->curl);
	if (gcal_obj->auth)
		free(gcal_obj->auth);

}


static size_t write_cb(void *ptr, size_t count, size_t chunk_size, void *data)
{

	size_t size = count * chunk_size;
	struct gcal_resource *gcal_ptr = (struct gcal_resource *) data;

	if (size > gcal_ptr->length) {
		/* TODO: I think this maybe can go to a distinct function
		 * if we are going to it in several points in the code
		 */
		free(gcal_ptr->buffer);
		gcal_ptr->length = size + 1;
		gcal_ptr->buffer = (char *) malloc(gcal_ptr->length);

		if (!gcal_ptr->buffer) {
			fprintf(stderr, "write_cb: Failed relloc\n");
			goto exit;
		}

	}

	strncpy(gcal_ptr->buffer, (char *)ptr, gcal_ptr->length);

exit:
	return size;
}

int gcal_get_authentication(char *user, char *password,
			    struct gcal_resource *ptr_gcal)
{
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	int post_len = 0;
	char *post = NULL;
	int result = -1;
	long request_stat;
	int count = 0;
	char *ptr = NULL;

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
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_HTTPHEADER,
			 response_headers);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_URL, GCAL_URL);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_POSTFIELDS, post);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(ptr_gcal->curl, CURLOPT_WRITEDATA,
			 (void *)ptr_gcal);

	res = curl_easy_perform(ptr_gcal->curl);

	curl_easy_getinfo(ptr_gcal->curl, CURLINFO_HTTP_CODE,
			  &request_stat);
	if (!res && (request_stat == GCAL_DEFAULT_ANSWER))
		result = 0;
	else
		fprintf(stderr, "%s\n", curl_easy_strerror(res));

	if (post)
		free(post);

	/* gcalendar server returns a string like this:
	 * SID=value\n
	 * LSID=value\n
	 * Auth=value\n
	 * and we only need the authorization token to login later.
	 * TODO: treat error if 'strdup' fails.
	 */
	if (ptr_gcal->auth)
		free(ptr_gcal->auth);
	ptr = ptr_gcal->buffer;
	while ((ptr = strchr(ptr, HEADER_BREAK))) {
		++count;
		++ptr;
		if (count == 2)
			ptr_gcal->auth = strdup(ptr + sizeof("Auth=") - 1);
	}

exit:
	return result;

}

