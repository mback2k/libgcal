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
 * @file   internal_gcal.h
 * @author Adenilson Cavalcanti
 * @date   Fri May 30 16:46:00 2008
 *
 * @brief  Internal gcal resource structure definition. The user shalt not
 * mess with it.
 *
 * I got to move it to a distinct file to share it between gcal.c and
 * gcontact.h.
 */

#ifndef __INTERNAL_GCAL__
#define __INTERNAL_GCAL__

#include <curl/curl.h>
#include "gcal_parser.h"

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
	/** Google service choose, see \ref service  */
	char service[3];

};

#endif
