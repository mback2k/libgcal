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

/** Since user cannot create an static instance of it, it entitles itself
 * to be a completely abstract data type. See \ref gcal_event.
 */
typedef struct gcal_event* gcal_event_t;


/** Structure to hold events array, use it as a parameter for
 * \ref gcal_get_events.
 *
 * This structure will contain the number of retrieved entries, as also
 * the pointer to an array of \ref gcal_event structures.
 */
struct gcal_event_array {
	/** See \ref gcal_event. */
	gcal_event_t entries;
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
 * @param raw_xml A string with google data XML of this entry.
 *
 * @return A gcal_event_t object on success or NULL otherwise.
 */
gcal_event_t gcal_event_new(char *raw_xml);

/** Free an gcal event object.
 *
 *
 * @param event An gcal event object, see also \ref gcal_event_new.
 */
void gcal_event_delete(gcal_event_t event);


/** Helper function, does all calendar events dump and parsing, returning
 * the data as an array of \ref gcal_event. See too \ref gcal_event_array.
 *
 * @param gcalobj A libgcal object, must be previously authenticated with
 * \ref gcal_get_authentication.
 *
 * @param events_array Pointer to an events array structure. See
 * \ref gcal_event_array.
 *
 * @return 0 on success, -1 otherwise.
 */
int gcal_get_events(gcal_t gcalobj, struct gcal_event_array *events_array);


/** Use this function to cleanup an array of calendar events.
 *
 * See also \ref gcal_get_events.
 *
 * @param events A pointer to an events array structure. See
 * \ref gcal_event_array.
 */
void gcal_cleanup_events(struct gcal_event_array *events);

/** Create a new event in user's calendar.
 *
 * You should have authenticate before using \ref gcal_get_authentication.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param event An event object, see \ref gcal_event_new.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_add_event(gcal_t gcal_obj, gcal_event_t event);

/** Updates an already existant event.
 *
 * Use it to update an event, but pay attention that you neeed to have
 * a valid event object (i.e. that has at least the edit_url to this entry).
 * See also \ref gcal_get_edit_url and \ref gcal_event_get_xml).
 *
 * A common use case is when you added a new event using \ref gcal_add_event
 * and later whant to edit it.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param event An event object.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_update_event(gcal_t gcal_obj, gcal_event_t event);

/** Deletes an event (pay attention that google server will mark this
 * event as 'cancelled').
 *
 * Being cancelled still makes possible to retrieve all the data of this
 * event (for a timeframe of a few weeks).
 *
 * The event object must be valid (i.e. has at least the edit_url to this
 * entry). See also \ref gcal_get_edit_url and \ref gcal_event_get_xml).
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param event An event object.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_erase_event(gcal_t gcal_obj, gcal_event_t event);

/** Query for updated events (added/edited/deleted).
 *
 * Its going to retrieve only updated events from user's calendar, its
 * useful if you implementing sync software and have downloaded all events
 * at least one time.
 *
 * Use this function to get only the changed data.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param events A pointer to a struct of type \ref gcal_event_array.
 *
 * @param timestamp A timestamp in format RFC 3339 format
 * (e.g. 2008-09-10T21:00:00Z) (see \ref TIMESTAMP_MAX_SIZE and
 * \ref get_mili_timestamp). It can include timezones too.
 * If you just want to get the updated events starting from today at 06:00Z,
 * use NULL as parameter.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_get_updated_events(gcal_t gcal_obj, struct gcal_event_array *events,
			    char *timestamp);


/** Helper function, extracts the edit URL in a XML entry
 *
 * The use of this function is a corner case: say that you just added an event
 * (so you dont have the gevent object for some reason) and want to load a
 * completely different event (say from a XML file) and want to update the
 * event with the new data.
 * You need to being able to access the older edit_url from the raw XML (that
 * I hope its still available somewhere) and that is when this function can
 * save your ass.
 *
 * A better approach would just create a new event object out from the older
 * XML data and use \ref gcal_event_get_url.
 * \todo: revise the real need of this function (at least as a public function).
 *
 * @param entry A pointer to a string that represents an event as raw XML.
 *
 * @param extracted_url This will be loaded with the edit_url, remember to free
 * it up.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_get_edit_url(char *entry, char **extracted_url);

/** Helper function, extracts the ETag code in a XML entry, required by
 * (Google Data API 2.0).
 *
 * The use of this function is a corner case: say that you just added an event
 * (so you dont have the gevent object for some reason) and want to load a
 * completely different event (say from a XML file) and want to update the
 * event with the new data.
 * You need to being able to access the older ETag from the raw XML (that
 * I hope its still available somewhere) and that is when this function can
 * save your ass.
 *
 * A better approach would just create a new event object out from the older
 * XML data and use \ref gcal_event_get_url. See also \ref gcal_get_edit_url.
 * \todo: revise the real need of this function (at least as a public function).
 *
 * @param entry A pointer to a string that represents an event as raw XML.
 *
 * @param extracted_etag This will be loaded with the edit_url, remember to free
 * it up.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_get_extract_etag(char *entry, char **extracted_etag);

/* Raw XML base functions: common for both calendar/contacts */

/** Adds a new entry (event or contact) in to user's feed, using raw xml.
 *
 * Use this if you already have the input data as a valid atom entry
 * XML data.  You don't need to create an entry object (gcal_event or
 * gcal_contact).
 *
 * A context where this function is handy is if you are going to load the
 * entries from XML files. Another context is if you use XSLT to convert
 * from another XML format (e.g. opensync) to create the gdata format.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param xml_entry A pointer to a string with XML representing the entry.
 *
 * @param xml_updated A pointer to pointer to string, it will have the updated
 * entry with new fields (updated, edit_url, ID). Remember to clean up this
 * memory. If you don't care about getting this information, just pass NULL.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_add_xmlentry(gcal_t gcal_obj, char *xml_entry, char **xml_updated);


/** Updates an entry (event or contact) using raw xml.
 *
 * For rationale of xml functions, see \ref gcal_add_xmlentry. Use it to update
 * (i.e. change) an entry.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param xml_entry A pointer to a string with XML representing the entry.
 *
 * @param xml_updated A pointer to pointer to string, it will have the updated
 * entry with new fields (updated, edit_url, ID). Remember to clean up this
 * memory. If you don't care about getting this information, just pass NULL.
 *
 * @param edit_url Common case, pass NULL (it assumes that 'xml_entry' has
 * a valid edit_url field). Case negative, you can supply the edit_url using
 * this parameter.
 *
 * @param etag Common case, pass NULL (it assumes that 'xml_entry' has
 * a valid etag field). Case negative, you can supply the etag using
 * this parameter. Required by Google Data protocols 2.0.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_update_xmlentry(gcal_t gcal_obj, char *xml_entry, char **xml_updated,
			 char *edit_url, char *etag);


/** Deletes an entry (event or contact) using raw xml.
 *
 * For rationale of xml functions, see \ref gcal_add_xmlentry. Use it to update
 * (i.e. change) an entry.
 *
 * @param gcal_obj A gcal object, see \ref gcal_new.
 *
 * @param xml_entry A pointer to a string with XML representing the entry. It
 * has to have a valid edit_url field or operation will fail (i.e. you must
 * have created this xml out of an pre-existant entry).
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_erase_xmlentry(gcal_t gcal_obj, char *xml_entry);


/** Returns an event element from an event array.
 *
 * Since to final user events are abstract types, even if is possible to access
 * internal \ref gcal_event_array vector of events, its not possible to do
 * pointer arithmetic with it. Use this function as an
 * accessor to them.
 * A context where this function is useful is when downloading all events
 * from user calendar using \ref gcal_get_events.
 *
 * @param events An array of events, see \ref gcal_event_array.
 *
 * @param _index Index of element (is zero based).
 *
 * @return Either a pointer to the event object or NULL.
 */
gcal_event_t gcal_event_element(struct gcal_event_array *events, size_t _index);


/* Here starts gcal_event accessors */

/** Access event ID.
 *
 * Each entry has a unique ID assigned by google server.
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_id(gcal_event_t event);

/** Access last updated timestamp.
 *
 * Each operation (add/edit/delete) will set a timestamp to the entry.
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_updated(gcal_event_t event);

/** Access event title.
 *
 * All entries have a title, with semantic depending on the entry type:
 * events (event title) or contact (contact name).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_title(gcal_event_t event);

/** Access the edit_url field.
 *
 * All entries have an edit_url field (which is the combo of ID+cookie) that
 * must be used to do operations (edit/delete).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_url(gcal_event_t event);

/** Access the etag field.
 *
 * All entries have an etag field (required by Google Data API 2.0) that
 * must be used to do operations (edit/delete).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_etag(gcal_event_t event);

/** Access the raw XML representation of the entry.
 *
 * Besides having the more important information already parsed, its still
 * possible to access the raw xml of the entry if and *only* if you set
 * this mode in \ref gcal_t object using \ref gcal_set_store_xml function
 * *before* getting the data (using \ref gcal_get_events).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set).
 */
char *gcal_event_get_xml(gcal_event_t event);


/** Checks if the current event was deleted or not.
 *
 * When parsing the entry, the respective element used to represent deleted or
 * cancelled status (for calendar events) is stored internally of event object.
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return 1 for deleted, 0 for not deleted, -1 for error case (f the event
 * object is invalid).
 */
char gcal_event_is_deleted(gcal_event_t event);


/* This are the fields unique to calendar events */

/** Access event description.
 *
 * Events have an extra field for description (like: "Meeting with someone and
 * remember to discuss whatever targeting do something").
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_content(gcal_event_t event);

/** Checks if this is a recurrent event.
 *
 * Case positive, it will return the string with the representation of recurrence
 * rule. Google calendar uses a subset of iCalendar to represent this (yeah, brain
 * f*ked in my opinion put another text format inside of XML...). See more
 * information here: http://code.google.com/apis/calendar/developers_guide_protocol.html#CreatingRecurring
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_recurrent(gcal_event_t event);

/** Access event start timestamp.
 *
 * Google calendar uses RFC 3339 format (e.g. "2008-09-11T21:00:00Z") to represent
 * when an event will start. It can have the timezone information too
 * (e.g. "2008-09-11T12:39:00-04:00" for a -4hours from UTC timezone, a.k.a. Manaus).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_start(gcal_event_t event);

/** Access event end timestamp.
 *
 * See too \ref gcal_event_get_start for further discussion.
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_end(gcal_event_t event);

/** Access event location/place.
 *
 * Google calendar allows to store any string as the place where the event
 * will happen (even GPS coordinates).
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_where(gcal_event_t event);


/** Access event status.
 *
 * An event can have some status (confirmed/cancelled) and its possible to
 * access then. For while, libgcal store the whole URL with the description
 * of event status (e.g. "http://schemas.google.com/g/2005#event.confirmed").
 * \todo Use enumeration to represent status.
 *
 * @param event An event object, see \ref gcal_event.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_event_get_status(gcal_event_t event);


/* Here starts the setters */

/** Sets event title.
 *
 * Use this to assign an event title (or to change it if you wish to update an
 * already existant calendar event).
 *
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field String with event title (e.g. "Dinner with Mary").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_title(gcal_event_t event, const char *field);

/** Set event detailed description.
 *
 * See also \ref gcal_event_get_content for an example of valid description.
 *
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field A string with event description.
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_content(gcal_event_t event, const char *field);

/** Set event start time.
 *
 * See also \ref gcal_event_get_start.
 *
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field A string with the timestamp for start time using RFC 3339 format
 * (e.g. "2008-09-11T21:00:00Z").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_start(gcal_event_t event, const char *field);


/** Set event end time.
 *
 * See also \ref gcal_event_set_start.
 *
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field A string with the timestamp for start time using RFC 3339 format
 * (e.g. "2008-09-11T22:00:00Z").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_end(gcal_event_t event, const char *field);

/** Set event location/place.
 *
 * See also \ref gcal_event_get_where.
 *
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field The place where event is supposed to happen (e.g. "Nearby an
 * nice pub, close the church")
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_where(gcal_event_t event, const char *field);

/* TODO: Not implemented */

/** Sets recurrence rule.
 *
 * \todo implement this method, its just stub now.
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field A string with google invalid iCalendar format
 * to represent recurrence rules.
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_recurrent(gcal_event_t event, const char *field);

/** Set event status (for while, all then were created as 'confirmed').
 *
 * \todo implement this method, its just stub now.
 * @param event An event object, see \ref gcal_event and \ref gcal_event_new.
 *
 * @param field Status description (should I use enums or string?).
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_status(gcal_event_t event, const char *field);

/** Sets event edit url.
 *
 * This field is a hard requirement to edit/delete a event. Starting with
 * google data API 2.0, the ETag is also required.
 *
 * @param event A event object, see \ref gcal_event.
 *
 * @param field String with the edit url email
 * (e.g. "http://www.google.com/m8/feeds/events/user%40gmail.com/base/2").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_url(gcal_event_t event, const char *field);


/** Sets event ID.
 *
 * Each event has an ID (but this can extracted from the edit_url).
 *
 * @param event A event object, see \ref gcal_event.
 *
 * @param field String with event ID (e.g. "joe.doe@nobody.com").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_id(gcal_event_t event, const char *field);


/** Sets event ETag.
 *
 * Starting with google data API 2.0, the ETag is used for versioning the
 * entries. Is required for edit/delete.
 *
 * @param event A event object, see \ref gcal_event.
 *
 * @param field String with event ETag (e.g. "Q3c5eDVSLyp7ImA9WxRbFE0KRAY.").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_event_set_etag(gcal_event_t event, const char *field);




#endif
