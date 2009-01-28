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
 * @file   gcontact.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Thu Jun 26 07:25:35 2008
 *
 * @brief  libgcal contacts user public API.
 *
 * Use this functions to handle common tasks when dealing with google contacts.
 */

#ifndef __GCONTACT_LIB__
#define __GCONTACT_LIB__

#include "gcal.h"
#include "gcont.h"

/** Since user cannot create an static instance of it, it entitles itself
 * to be a completely abstract data type. See \ref gcal_contact.
 */
typedef struct gcal_contact* gcal_contact_t;

/** Contact entries array. Its used to hold retrieved contacts
 * retrieved from google server.
 */
struct gcal_contact_array {
	/** See \ref gcal_contact. */
	gcal_contact_t entries;
	/** The number of entries */
	size_t length;
};


/** Creates a new google contact object.
 *
 * If you are going to add new contact, see also \ref gcal_add_contact.
 *
 * @param raw_xml A string with google data XML of this entry.
 *
 * @return A gcal_contact object on success or NULL otherwise.
 */
gcal_contact_t gcal_contact_new(char *raw_xml);

/** Free a gcal contact object.
 *
 *
 * @param contact An gcal contact object, see also \ref gcal_contact_new.
 */
void gcal_contact_delete(gcal_contact_t contact);


/** Helper function, does all contact dump and parsing, returning
 * the data as an array of \ref gcal_contact.
 *
 * @param gcalobj A libgcal object, must be previously authenticated with
 * \ref gcal_get_authentication. See also \ref gcal_new.
 *
 * @param contact_array Pointer to a contact array structure. See
 * \ref gcal_contact_array.
 *
 * @return 0 on success, -1 otherwise.
 */
int gcal_get_contacts(gcal_t gcalobj, struct gcal_contact_array *contact_array);

/** Use this function to cleanup an array of contacts.
 *
 * See also \ref gcal_get_contacts.
 *
 * @param contacts A pointer to an contacts array structure. See
 * \ref gcal_contact_array.
 */
void gcal_cleanup_contacts(struct gcal_contact_array *contacts);


/** Returns a contact element from a contact array.
 *
 * Since to final user contacts are abstract types, even if is possible to
 * access internal \ref gcal_contact_array vector of contacts, its not
 * possible to do pointer arithmetic. Use this function as an
 * accessor to them.
 * A context where this function is useful is when downloading all contacts
 * from user account using \ref gcal_get_contacts.
 *
 * @param contacts An array of contacts, see \ref gcal_contact_array.
 *
 * @param _index Index of element (is zero based).
 *
 * @return Either a pointer to the event object or NULL.
 */
gcal_contact_t gcal_contact_element(struct gcal_contact_array *contacts,
				    size_t _index);

/** Add a new contat in user's account.
 *
 * You should have authenticate before using \ref gcal_get_authentication.
 *
 * @param gcalobj A gcal object, see \ref gcal_new.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_add_contact(gcal_t gcalobj, gcal_contact_t contact);


/** Updates an already existant contact.
 *
 * Use it to update a contact, but pay attention that you neeed to have
 * a valid contact object (i.e. that has at least the edit_url to this entry).
 * See also \ref gcal_get_edit_url and \ref gcal_contact_get_xml).
 *
 * A common use case is when you added a new contact using \ref gcal_add_contact
 * and later whant to edit it.
 *
 * @param gcalobj A gcal object, see \ref gcal_new.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_update_contact(gcal_t gcalobj, gcal_contact_t contact);


/** Deletes a contact (once you do this, is not possible to recover the
 * information from this contact, only the ID).
 *
 * The behavior is different from calendar events, where is possible to
 * retrieve all the data from a deleted event.
 *
 * @param gcalobj A gcal object, see \ref gcal_new.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 0 on sucess, -1 otherwise.
 */
int gcal_erase_contact(gcal_t gcalobj, gcal_contact_t contact);

/** Query for updated contacts (added/edited/deleted).
 *
 * Pay attention that by default, google server will hide deleted contacts.
 * If you need to access them, remember to set it using \ref gcal_deleted
 * *before* requesting the data.
 *
 * Use this function to get only the changed data. Somewhat related, see too
 * \ref gcal_get_updated_events.
 *
 * @param gcal_obj A libgcal object, must be previously authenticated with
 * \ref gcal_get_authentication. See also \ref gcal_new.
 *
 * @param contacts Pointer to a contact array structure. See
 * \ref gcal_contact_array.
 *
 * @param timestamp A timestamp in format RFC 3339 format
 * (e.g. 2008-09-10T21:00:00Z) (see \ref TIMESTAMP_MAX_SIZE and
 * \ref get_mili_timestamp). It can include timezones too.
 * If you just want to get the updated events starting from today at 06:00Z,
 * use NULL as parameter.
 *
 * @return 0 on success, -1 otherwise.
 */
int gcal_get_updated_contacts(gcal_t gcal_obj,
			      struct gcal_contact_array *contacts,
			      char *timestamp);


/* Here starts gcal_contact accessors */

/** Access contact ID.
 *
 * Each entry has a unique ID assigned by google server.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_id(gcal_contact_t contact);

/** Access last updated timestamp.
 *
 * Not only each operation will change the updated timestamp of a contact,
 * but I *guess* that (in the case of a google account contact) your
 * contact can change this too.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_updated(gcal_contact_t contact);

/** Access contact name.
 *
 * All entries have a title, with semantic depending on the entry type:
 * events (event title) or contact (contact name).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_title(gcal_contact_t contact);

/** Access the edit_url field.
 *
 * All entries have an edit_url field (which is the combo of ID+cookie) that
 * must be used to do operations (edit/delete).
 * See also \ref gcal_get_edit_url.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_url(gcal_contact_t contact);


/** Access the ETag
 *
 * All entries have an etag field (Google Data API 2.0) that must be used
 * to do operations (edit/delete). See also \ref gcal_get_edit_etag.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_etag(gcal_contact_t contact);

/** Access the raw XML representation of the entry.
 *
 * Besides having the more important information already parsed, its still
 * possible to access the raw xml of the entry if and *only* if you set
 * this mode in \ref gcal_t object using \ref gcal_set_store_xml function
 * *before* getting the data (using \ref gcal_get_contacts).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_xml(gcal_contact_t contact);

/** Checks if the current event was deleted or not.
 *
 * When parsing the entry, the respective element used to represent deleted
 * is stored internally of contact object.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 1 for deleted, 0 for not deleted, -1 for error case (f the event
 * object is invalid).
 */
char gcal_contact_is_deleted(gcal_contact_t contact);


/* This are the fields unique to contacts */

/** Access contact e-mail.
 *
 * Email has an important rule for google contacts, since its the only
 * really required field to being able to add a new entry in user's
 * contact list.
 * It needs to be unique, no 2 contacts can have the same e-mail.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").

 */
char *gcal_contact_get_email(gcal_contact_t contact);

/** Access contact description.
 *
 * This the place where contacts notes can be retrieved.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_content(gcal_contact_t contact);

/** Missing implementation.
 *
 * \todo Implement retrieve of extra fields in \ref atom_parser.c
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Will only return NULL.
 */
char *gcal_contact_get_orgname(gcal_contact_t contact);

/** Missing implementation.
 *
 * \todo Implement retrieve of extra fields in \ref atom_parser.c
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Will only return NULL.
 */
char *gcal_contact_get_orgtitle(gcal_contact_t contact);

/** Missing implementation.
 *
 * \todo Implement retrieve of extra fields in \ref atom_parser.c
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Will only return NULL.
 */
char *gcal_contact_get_im(gcal_contact_t contact);

/** Missing implementation.
 *
 * \todo Implement retrieve of extra fields in \ref atom_parser.c
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Will only return NULL.
 */
char *gcal_contact_get_phone(gcal_contact_t contact);

/** Missing implementation.
 *
 * \todo Implement retrieve of extra fields in \ref atom_parser.c
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Will only return NULL.
 */
char *gcal_contact_get_address(gcal_contact_t contact);


/* Here starts the gcal_contact setters */

/** Sets contact name.
 *
 * Use this to assign a contact's name (or to change it if you wish to update an
 * already existant contact).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field String with contact name (e.g. "Joe Doe").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_contact_set_title(gcal_contact_t contact, char *field);

/** Sets contac email.
 *
 * This field is a hard requirement to create a new contact. Google server
 * supports more e-mails with special tags too, but its not supported
 * for while using gcal_contact object.
 *
 * If you need to add a contact entry with all optional fields, an
 * alternative is to use \ref gcal_add_xmlentry.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field String with contact email (e.g. "joe.doe@nobody.com").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_contact_set_email(gcal_contact_t contact, char *field);

/* TODO: Contacts extra fields, not implemented in internal functions
 * see ticket: http://code.google.com/p/libgcal/issues/detail?id=4
 */
/** Missing implementation.
 *
 * \todo Implement setting extra fields.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Phone number.
 *
 * @return Will only return -1.
 */
int gcal_contact_set_phone(gcal_contact_t contact, char *field);


#endif
