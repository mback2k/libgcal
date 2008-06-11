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
 * @file   gcal.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 20:15:09 2008
 *
 * @brief  libgcal header file.
 *
 * This defines the public functions.
 *
 */
#ifndef __GCAL_LIB__
#define __GCAL_LIB__

/* For size_t */
#include <stdlib.h>

/** Google service that user will contact */
typedef enum { GCALENDAR, GCONTACT } gservice;

/** Upload (being POST or PUT) definition option.
 *
 * Its used to set behavior in \ref up_entry function.
 */
typedef enum {
	/** Code for HTTP POST. */
	POST,
	/** Code for HTTP PUT */
	PUT } HTTP_CMD;


/** Library structure. It holds resources (curl, buffer, etc).
 */
struct gcal_resource;

/** Library structure, represents each calendar event entry.
 */
struct gcal_entries {
	/** The 'what' field */
	char *title;
	/** element ID */
	char *id;
	/** The edit URL */
	char *edit_uri;
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
	/** Time when the event was updated. */
	char *updated;
};

/** Library structure destructor (use it free its internal resources properly).
 */
void gcal_destroy(struct gcal_resource *gcal_obj);


/** Internal use function, cleans up the internal buffer.
 *
 * \todo move it to a distinct internal module.
 *
 * @param gcal_obj Library resource structure pointer.
 */
void clean_buffer(struct gcal_resource *gcal_obj);


/** Library structure constructor, the user can only have pointers to the
 * library \ref gcal_resource structure.
 *
 * Concerning google service type, it defaults to google calendar. You can
 * change it using \ref gcal_set_service.
 *
 * @param gservice Service that libgcal will handle (for while, GCALENDAR
 * and GCONTACT).
 *
 * @return A pointer to a newly created object or NULL.
 */
struct gcal_resource *gcal_initialize(gservice mode);

/** Sets the google service that user wants to authenticate.
 *
 * For while, only calendar (cl) and contacts (cp) are supported.
 *
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource.
 *
 * @param option Service type, see \ref service.
 */
void gcal_set_service(struct gcal_resource *ptr_gcal, gservice mode);


/** Gets from google an authentication token, using the 'ClientLogin' service.
 *
 * Use it before getting/setting google calendar events.
 *
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource
 *
 * @param user The user google login account.
 *
 * @param password User password in plain text
 *                 \todo think in a way to encrypt password
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_get_authentication(struct gcal_resource *ptr_gcal,
			    char *user, char *password);


/** Dumps events from default calendar to internal buffer.
 * \todo Let the library user select which calendar he/she wants
 * to get the events. See \ref gcal_calendar_list.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_dump(struct gcal_resource *ptr_gcal);

/** Get a list of users calendars (gcalendar supports multiple calendars
 * besides the default calendar).
 *
 * I think it would be a good idea to let the library user decide which
 * calendar to get the events. See too \ref gcal_dump.
 *
 * \todo Parse the Atom feed and provide easy access to the calendar lists.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_calendar_list(struct gcal_resource *ptr_gcal);


/** Return the number of event entries a calendar has (you should
 * had got the atom stream before, using \ref gcal_dump).
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return -1 on error, any number >= 0 otherwise.
 */
int gcal_entries_number(struct gcal_resource *ptr_gcal);


/** Extracts from the atom stream the calendar event entries (you should
 * had got the atom stream before, using \ref gcal_dump).
 *
 * Pay attention that it returns a vector of structures that must be destroyed
 * using \ref gcal_destroy_entries.
 *
 * Since atom XML feeds can get huge, as soon the function creates entries
 * vector and copies the data from the internal \ref gcal_resource buffer,
 * it will free its internal buffer to save memory.
 *
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param length Pointer to an unsigned int, it will have the vector length.
 *
 * @return A pointer on sucess, NULL otherwise.
 */
struct gcal_entries *gcal_get_entries(struct gcal_resource *ptr_gcal,
				      size_t *length);

/** Cleanup memory of 1 entry structure pointer.
 *
 *
 * @param entry A pointer to a \ref gcal_entries.
 */
void gcal_destroy_entry(struct gcal_entries *entry);

/** Cleanup the memory of a vector of calendar entries created using
 * \ref gcal_get_entries.
 *
 * @param entries A pointer to a vector of \ref gcal_entries structure.
 *
 * @param length The vector length.
 */
void gcal_destroy_entries(struct gcal_entries *entries, size_t length);

/** Posts data to a server URL and checks its result.
 *
 * I'm not sure if this one should be here, since it is a more an internal
 * function (but I need to share it with 'contacts' too).
 *
 * \todo move it to a distinct internal module.
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param url The server URL.
 * @param header First header part.
 * @param header2 Second header part (can be NULL).
 * @param header3 Third header part (can be NULL).
 * @param post_data The data to post (can be NULL).
 * @param expected_answer The expected answer code, see GCAL_DEFAULT_ANSWER and
 * friends.
 *
 * @return -1 on error, 0 on success.
 */
int http_post(struct gcal_resource *ptr_gcal, const char *url,
	      char *header, char *header2, char *header3,
	      char *post_data, const int expected_answer);


/** Uploads an entry (calendar or contact) to a server URL
 *
 * Used by \ref gcal_create_event, \ref gcal_create_contact,
 * \ref gcal_edit_event.
 *
 *
 * @param data2post A pointer to string, it will be the body to be posted.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param url_server The URL of server (one for calendar and other for
 * contacts).
 *
 * @param up_mode If the upload of data will be using PUT or POST (internally
 * it uses 'http_put' and \ref 'http_post').
 *
 * @param expected_code The expected return code from server (200, 201, etc.)
 * See GCAL_DEFAULT_ANSWER and friends.
 *
 * @return -1 on error, 0 on success.
 */
int up_entry(char *data2post, struct gcal_resource *ptr_gcal,
	     const char *url_server, HTTP_CMD up_mode, int expected_code);


/** Creates an new calendar event.
 *
 * You need to first succeed to get an authorization token using
 * \ref gcal_get_authentication.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param entries A pointer to an calendar entry event (see \ref gcal_entries).
 *
 * @param updated Pass a pointer to a \ref gcal_entries structure if you
 * wish to access the newly created contact (i.e. access fields like
 * edit_uri and id). If you don't need it, just pass NULL.
 *
 * @return -1 on error, 0 on success, -2 if operation went correctly but
 * cannot return 'updated' entry.
 */
int gcal_create_event(struct gcal_resource *ptr_gcal,
		      struct gcal_entries *entries,
		      struct gcal_entries *updated);

/** Deletes a calendar event.
 *
 * You need to first succeed to get an authorization token using
 * \ref gcal_get_authentication.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param entry A pointer to one calendar entry event (see \ref gcal_entries).
 *
 * @return -1 on error, 0 on success.
 */
int gcal_delete_event(struct gcal_resource *ptr_gcal,
		      struct gcal_entries *entry);


/** Edits a calendar event.
 *
 * It requires the presence of field 'edit_uri' in entry pointer structure
 * (see \ref gcal_entries).
 *
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param entry A pointer to an calendar entry event.
 *
 * @param updated Pass a pointer to a \ref gcal_entries structure if you
 * wish to access the newly created contact (i.e. access fields like
 * edit_uri and id). If you don't need it, just pass NULL.
 *
 * @return -1 on error, 0 on success, -2 if operation went correctly but
 * cannot return 'updated' entry.
 */
int gcal_edit_event(struct gcal_resource *ptr_gcal,
		    struct gcal_entries *entry,
		    struct gcal_entries *updated);



/** Provides access to internal buffer (e.g. to access the raw Atom stream).
 *
 * Pay attention that you *must not* mess with its memory, since when deleting
 * the gcal_resource pointer with \ref gcal_destroy the memory pointed will
 * be freed.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return A pointer to internal gcal_resource buffer.
 */
char *gcal_access_buffer(struct gcal_resource *ptr_gcal);


#endif
