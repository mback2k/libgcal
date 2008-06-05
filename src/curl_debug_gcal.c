#include <stdio.h>
#include "curl_debug_gcal.h"

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


static void curl_debug_dump(const char *text,
		 FILE *stream, unsigned char *ptr, size_t size,
		 char nohex)
{
	size_t i;
	size_t c;

	unsigned int width=0x10;

	/* without the hex output, we can fit more on screen */
	if (nohex)
		width = 0x40;

	fprintf(stream, "%s, %zd bytes (0x%zx)\n", text, size, size);

	for (i = 0; i< size; i+= width) {

		fprintf(stream, "%04zx: ", i);

		if (!nohex) {
			/* hex not disabled, show it */
			for(c = 0; c < width; c++)
				if(i+c < size)
					fprintf(stream, "%02x ", ptr[i+c]);
				else
					fputs("   ", stream);
		}

		for(c = 0; (c < width) && (i+c < size); c++) {
			/* check for 0D0A; if found, skip past and start
			 * a new line of output
			 */
			if (nohex && (i+c+1 < size) &&
			    ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
				i+=(c+2-width);
				break;
			}

			fprintf(stream, "%c",
				(ptr[i+c]>=0x20) &&
				(ptr[i+c]<0x80)?ptr[i+c]:'.');

			/* check again for 0D0A, to avoid an extra
			 * \n if it's at width
			 */
			if (nohex && (i+c+2 < size) &&
			    ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
				i+=(c+3-width);
				break;
			}
		}

		fputc('\n', stream); /* newline */
	}

	fflush(stream);
}

int curl_debug_gcal_trace(CURL *handle, curl_infotype type,
			  unsigned char *data, size_t size,
			  void *userp)
{
	struct data_curl_debug *config = (struct data_curl_debug *)userp;
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	curl_debug_dump(text, stderr, data, size, config->trace_ascii);
	return 0;
}
