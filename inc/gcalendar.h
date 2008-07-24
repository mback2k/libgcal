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
 * @file   gcalendar.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Tue Jun 24 16:17:25 2008
 *
 * @brief  libgcal calendar user public API.
 *
 * Use this functions to handle common tasks when dealing with google calendar.
 */

#ifndef __GCALENDAR_LIB__
#define __GCALENDAR_LIB__

#include "gcal.h"

/** Structure to hold events array, use it as a parameter for
 * \ref gcal_get_events.
 *
 * This structure will contain the number of retrieved entries, as also
 * the pointer to an array of \ref gcal_event structures.
 */
struct gcal_event_array {
	/** See \ref gcal_event. */
	struct gcal_event *entries;
	/** The number of entries */
	size_t length;
};

/** Creates a new gcal object (you need then to talk with google
 * servers).
 *
 * @param mode Which google service to handle (GCONTACT and GCALENDAR)
 *
 * @return The new object on success or NULL otherwise.
 */
gcal_t gcal_new(gservice mode);

/** Deletes a gcal object.
 *
 * @param gcal_obj A gcal object created with \ref gcal_new.
 */
void gcal_delete(gcal_t gcal_obj);

/** Creates a new google calendar event object.
 *
 * If you are going to add new event, see also \ref gcal_add_event.
 *
 * @return A gcal_event object on success or NULL otherwise.
 */
gcal_event gcal_event_new(void);

/** Free an gcal event object.
 *
 *
 * @param event An gcal event object, see also \ref gcal_event_new.
 */
void gcal_event_delete(gcal_event event);


/** Helper function, does all calendar events dump and parsing, returning
 * the data as an array of \ref gcal_event.
 *
 * @param gcalobj A libgcal object, must be previously authenticated with
 * \ref gcal_get_authentication.
 *
 * @param events_array Pointer to an events array structure. See
 * \ref gcal_entry_array.
 *
 * @return 0 on success, -1 otherwise.
 */
int gcal_get_events(gcal_t gcalobj, struct gcal_event_array *events_array);


/** Use this function to cleanup an array of calendar events.
 *
 * See also \ref gcal_get_events.
 *
 * @param events A pointer to an events array structure. See
 * \ref gcal_entry_array.
 */
void gcal_cleanup_events(struct gcal_event_array *events);


int gcal_add_event(gcal_t gcal_obj, gcal_event event);
int gcal_update_event(gcal_t gcal_obj, gcal_event event);
int gcal_erase_event(gcal_t gcal_obj, gcal_event event);
int gcal_get_updated_events(gcal_t gcal_obj, struct gcal_event_array *events,
			    char *timestamp);


/* Raw XML base functions: common for both calendar/contacts */
int gcal_add_xmlentry(gcal_t gcal_obj, char *xml_entry);
int gcal_update_xmlentry(gcal_t gcal_obj, char *xml_entry);
int gcal_erase_xmlentry(gcal_t gcal_obj, char *xml_entry);


gcal_event gcal_event_element(struct gcal_event_array *events, size_t _index);


/* Here starts gcal_event accessors */
char *gcal_event_get_id(gcal_event event);
char *gcal_event_get_updated(gcal_event event);
char *gcal_event_get_title(gcal_event event);
char *gcal_event_get_url(gcal_event event);
char *gcal_event_get_xml(gcal_event event);


/* This are the fields unique to calendar events */
char *gcal_event_get_content(gcal_event event);
char *gcal_event_get_recurrent(gcal_event event);
char *gcal_event_get_start(gcal_event event);
char *gcal_event_get_end(gcal_event event);
char *gcal_event_get_where(gcal_event event);
char *gcal_event_get_status(gcal_event event);


/* Here starts the setters */
int gcal_event_set_title(gcal_event event, char *field);
int gcal_event_set_content(gcal_event event, char *field);
int gcal_event_set_start(gcal_event event, char *field);
int gcal_event_set_end(gcal_event event, char *field);
int gcal_event_set_where(gcal_event event, char *field);

/* TODO: Not implemented */
int gcal_event_set_recurrent(gcal_event event, char *field);
int gcal_event_set_status(gcal_event event, char *field);

#endif
