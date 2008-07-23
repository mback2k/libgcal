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
 * @brief  Internal gcal resource structure and constants definition.
 * The user shalt not mess with it.
 *
 * I got to move it to a distinct file to share it between gcal.c and
 * gcontact.h.
 */

#ifndef __INTERNAL_GCAL__
#define __INTERNAL_GCAL__

#include <curl/curl.h>
#include <libxml/parser.h>

/** Abstract type to represent a DOM xml tree (a thin layer over xmlDoc).
 */
typedef xmlDoc dom_document;

static const char GCAL_URL[] = "https://www.google.com/accounts/ClientLogin";
static const char GCAL_LIST[] = "http://www.google.com/calendar/feeds/"
	"default/allcalendars/full";
/* Google calendar URL for posting new events */
static const char GCAL_EDIT_URL[] = "http://www.google.com/calendar/feeds"
	"/default/private/full";
/* Google contacts URL for posting new contacts */
static const char GCONTACT_EDIT_START[] = "http://www.google.com/m8/feeds/"
	"contacts/";
static const char GCONTACT_EDIT_END[] = "%40gmail.com/full";

/* Google calendar query URL */
static const char GCAL_EVENT_START[] = "http://www.google.com/calendar/feeds/";
static const char GCAL_EVENT_END[] = "%40gmail.com/private/full";

/* Google contact query URL */
static const char GCONTACT_START[] = "http://www.google.com/m8/feeds/contacts/";
static const char GCONTACT_END[] = "%40gmail.com/base";

/* Google 'pages' results in a range pages of 25 entries. But for downloading
 * all results its requirement to set a 'good enough' upper limit of range of
 * entries. A hack to make 'gcal_dump' work.
 */
static const char GCAL_UPPER[] = "max-results=999999999";

static const int GCAL_DEFAULT_ANSWER = 200;
static const int GCAL_REDIRECT_ANSWER = 302;
static const int GCAL_EDIT_ANSWER = 201;
static const int GCAL_CONFLICT = 409;

static const char EMAIL_FIELD[] = "Email=";
static const char EMAIL_ADDRESS[] = "@gmail.com";
static const char PASSWD_FIELD[] = "Passwd=";
static const char SERVICE_FIELD[] = "service=";
static const char CLIENT_SOURCE[] = "source=libgcal";
static const char HEADER_AUTH[] = "Auth=";
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
	/** Google service choose, currently Calendar and contacts  */
	char service[3];
	/** HTTP code status from last request */
	long http_code;
	/** CURL error messages */
	char *curl_msg;
	/** Internal status from last request */
	int internal_status;
	/** Handler to internal logging file */
	FILE *fout_log;
	/** Max number of results (google pages its query results) */
	char *max_results;
	/** User defined timezone in RFC 3339 format: -/+hh:mm:ss */
	char *timezone;
	/** User defined location (used to define in which timezone the
	 * results will be returned). The format is
	 * "Continent/City_without_spaces", e.g. "America/Los_Angeles".
	 */
	char *location;
	/** Control if deleted entries will be returned or not (only
	 * valid for google contacts.
	 */
	int deleted;
	/** Controls if raw XML entries will be stored inside each
	 * event/contact object.
	 */
	char store_xml_entry;
};

/** This structure has the common data fields between google services
 * (calendar and contacts).
 */
struct gcal_entry {
	/** Controls if raw XML data will be stored. */
	char store_xml;
	/** element ID */
	char *id;
	/** Time when the event was updated. */
	char *updated;
	/** The 'what' field */
	char *title;
	/** The edit URL */
	char *edit_uri;
	/** RAW XML data of this entry */
	char *xml;
};

/** Library structure, represents each calendar event entry.
 */
struct gcal_event {
	/** Has the common entry data fields (id, updated, title, edit_uri) */
	struct gcal_entry common;
	/* Here starts calendar event unique fields */
	/** The event description */
	char *content;
	/** If the event is recurrent */
	char *dt_recurrent;
	/** When/start time */
	char *dt_start;
	/** When/end time */
	char *dt_end;
	/** Location of event */
	char *where;
	/** Event status */
	char *status;
};

/** Contact data type */
struct gcal_contact {
	/** Has the common entry data fields (id, updated, title, edit_uri) */
	struct gcal_entry common;
	/* Here starts google contact unique fields */
	/** Contact email */
	char *email;

	/* Here starts the extra fields */
	/** Notes about contact */
	char *content;
	/** Company name */
	char *org_name;
	/** Job title */
	char *org_title;
	/** IM contact */
	char *im;
	/** Phone number */
	char *phone_number;
	/** Address */
	char *post_address;

};

#endif
