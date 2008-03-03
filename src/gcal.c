/**
 * @file   gcal.c
 * @author teste
 * @date   Mon Mar  3 12:32:11 2008
 *
 * @brief  Base file for a gcalendar service access library.
 *
 * \todo:
 * - autotools buildsystem
 * - move the code to a library (with pkg-config support)
 * - write utest to invoque the library
 * - doxygen comments
 * - redirect HTTP output to a string buffer (currently it writes out
 * to stdout)
 * - write methods to access gcalendar events
 * - reuse the CURL structure (maybe a structure to hold resources?)
 * - think a way to securely store passwords
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>


static const char *GCAL_URL = "https://www.google.com/accounts/ClientLogin";


int gcal_get_authentication(char *user, char *password, char *auth)
{

	CURL *curl;
	CURLcode res;
	struct curl_slist *response_headers = NULL;
	/* FIXME: calculate size considering user + password */
	int post_len = 300;
	char *post = NULL;
	int result = -1;

	if (!(post = (char *) malloc(post_len)))
		goto exit;

	snprintf(post, post_len - 1, "Email=%s@gmail.com&Passwd=%s&"
		 "service=cl&source=libgcal", user, password);


	response_headers = curl_slist_append(response_headers,
					     "application/x-www-form-urlencoded"
					     );
	/* TODO: we should reuse this structure */
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER,
				 response_headers);
		curl_easy_setopt(curl, CURLOPT_URL, GCAL_URL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

		res = curl_easy_perform(curl);
		if (!res)
			result = 0;

	}


cleanup:
	curl_easy_cleanup(curl);
	if (post)
		free(post);

exit:
	return result;

}

/* int main(void) */
/* { */

/* 	int res; */
/* 	res = gcal_get_authentication("gcal4tester", "66libgcal", NULL); */
/* 	printf("result = %d\n", res); */

/* 	return 0; */
/* } */
