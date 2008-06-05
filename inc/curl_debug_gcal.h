#ifndef __DEBUG_CURL_GCAL__
#define __DEBUG_CURL_GCAL__

/* Coments: this came straight from libcurl examples directory and
 * will print information to stderr when executing curl_easy_perform.
 * I'm not completely sure about its author, but I'm assuming that it uses
 * the same license as curl itself (X11/MIT).
 *
 * I made only small changes in space and formating.
 *
 * Adenilson Cavalcanti
 */

/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: debug.c,v 1.2 2006-10-20 21:26:10 bagder Exp $
 */

#include <curl/curl.h>

struct data_curl_debug {
	char trace_ascii; /* 1 or 0 */
};


int curl_debug_gcal_trace(CURL *handle, curl_infotype type,
			  unsigned char *data, size_t size,
			  void *userp);


#endif
