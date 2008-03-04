/**
 * @file   gcal.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 12:32:11 2008
 *
 * @brief  Base file for a gcalendar service access library.
 *
 * \todo:
 * - redirect HTTP output to a string buffer (currently it writes out
 * to stdout)
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


struct gcal_resource {

	char *buffer;
	int length;
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

	ptr->length = 256;
	ptr->buffer = (char *) malloc(ptr->length);
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

	res = curl_easy_perform(ptr_gcal->curl);

	curl_easy_getinfo(ptr_gcal->curl, CURLINFO_HTTP_CODE,
			  &request_stat);
	if (!res && (request_stat == GCAL_DEFAULT_ANSWER))
		result = 0;
	else
		fprintf(stderr, "%s\n", curl_easy_strerror(res));

	if (post)
		free(post);

exit:
	return result;

}

