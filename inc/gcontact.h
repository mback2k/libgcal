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

typedef struct gcal_structured_subvalues *gcal_structured_subvalues_t;

/** Contact entries array. Its used to hold retrieved contacts
 * retrieved from google server.
 */
struct gcal_contact_array {
	/** See \ref gcal_contact. */
	gcal_contact_t entries;
	/** The number of entries */
	size_t length;
};

/** Phone number types allowed by Google API */
typedef enum {
	P_INVALID = -1,
	P_ASSISTANT,
	P_CALLBACK,
	P_CAR,
	P_COMPANY_MAIN,
	P_FAX,
	P_HOME,
	P_HOME_FAX,
	P_ISDN,
	P_MAIN,
	P_MOBILE,
	P_OTHER,
	P_OTHER_FAX,
	P_PAGER,
	P_RADIO,
	P_TELEX,
	P_TTY_TDD,
	P_WORK,
	P_WORK_FAX,
	P_WORK_MOBILE,
	P_WORK_PAGER,
	P_ITEMS_COUNT			// must be the last one!
} gcal_phone_type;

/** Email types allowed by Google API */
typedef enum {
	E_INVALID = -1,
	E_HOME,
	E_OTHER,
	E_WORK,
	E_ITEMS_COUNT			// must be the last one!
} gcal_email_type;

/** Address types allowed by Google API */
typedef enum {
	A_INVALID = -1,
	A_HOME,
	A_WORK,
	A_OTHER,
	A_ITEMS_COUNT			// must be the last one!
} gcal_address_type;

/** IM types allowed by Google API */
typedef enum {
	I_INVALID = -1,
	I_HOME,
	I_WORK,
	I_NETMEETING,
	I_OTHER,
	I_ITEMS_COUNT			// must be the last one!
} gcal_im_type;

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

/** Access contact e-mail address.
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

/** Access contact e-mail count.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of e-mail entries.
 */
int gcal_contact_get_emails_count(gcal_contact_t contact);

/** Access contact preferred e-mail.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of preferred e-mail.
 */
int gcal_contact_get_pref_email(gcal_contact_t contact);

/** Access contact e-mail address.
 *
 * @param contact A contact object, see \ref gcal_contact.
 * 
 * @param i Number of e-mail entry.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_email_address(gcal_contact_t contact, int i);

/** Access contact e-mail address type.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of e-mail entry.
 *
 * @return Type of e-mail.
 */
gcal_email_type gcal_contact_get_email_address_type(gcal_contact_t contact, int i);

/** Access contact e-mail address label.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of e-mail entry.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_email_address_label(gcal_contact_t contact, int i);

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

/** Access contact nickname.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return  Pointer to internal object field
 */
char *gcal_contact_get_nickname(gcal_contact_t contact);

/** Access contact organization name.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_organization(gcal_contact_t contact);

/** Access contact organization title/profission.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_profission(gcal_contact_t contact);

/** Access contact occupation/profession.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_occupation(gcal_contact_t contact);

/** Access contact website.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field
 */
char *gcal_contact_get_homepage(gcal_contact_t contact);

/** Access contact blog.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field
 */
char *gcal_contact_get_blog(gcal_contact_t contact);

/** Access contact telephone.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_phone(gcal_contact_t contact);

/** Access contact telephone count.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of telephone entries.
 */
int gcal_contact_get_phone_numbers_count(gcal_contact_t contact);

/** Access contact preferred telephone.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of preferred telephone.
 */
int gcal_contact_get_pref_phone_number(gcal_contact_t contact);

/** Access contact telephone.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of telephone entry.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_phone_number(gcal_contact_t contact, int i);

/** Access contact telephone type.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of telephone entry.
 *
 * @return Type of telephone.
 */
gcal_phone_type gcal_contact_get_phone_number_type(gcal_contact_t contact, int i);

/** Access contact phone number label.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of phone number entry.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_phone_number_label(gcal_contact_t contact, int i);

/** Access contact preferred IM address.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field.
 */
char *gcal_contact_get_im(gcal_contact_t contact);

/** Access contact IM count.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of IM entries.
 *
 */
int gcal_contact_get_im_count(gcal_contact_t contact);

/** Access contact preferred IM account.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of preferred IM account.
 *
 */
int gcal_contact_get_pref_im(gcal_contact_t contact);

/** Access contact IM protocol.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of the IM entry.
 *
 * @return Pointer to internal object field.
 */
char *gcal_contact_get_im_protocol(gcal_contact_t contact, int i);

/** Access contact IM address.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of the IM entry.
 *
 * @return Pointer to internal object field.
 */
char *gcal_contact_get_im_address(gcal_contact_t contact, int i);

/** Access contact IM label.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of the IM entry
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_im_label(gcal_contact_t contact, int i);

/** Access contact IM type.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param i Number of the IM entry.
 *
 * @return Type of the IM account.
 */
gcal_phone_type gcal_contact_get_im_type(gcal_contact_t contact, int i);

/** Access contact address (structuredPostalAddress.formattedAddress).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").

 */
char *gcal_contact_get_address(gcal_contact_t contact);

/** Access structured address objects.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field.
 */
gcal_structured_subvalues_t gcal_contact_get_structured_address(gcal_contact_t contact);

/** Access structured name object.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field.
 */
gcal_structured_subvalues_t gcal_contact_get_structured_name(gcal_contact_t contact);

/** Get one structured entry.
 *
 * @param structured_entry A structured entry object, see \ref gcal_structured_subvalues.
 *
 * @param structured_entry_nr Index of the entry.
 *
 * @param structured_entry_count Number of all entries.
 *
 * @param field_key Key of the structured entry.
 *
 * @return Pointer to internal object field.
 */
char *gcal_contact_get_structured_entry(gcal_structured_subvalues_t structured_entry, int structured_entry_nr, int structured_entry_count, const char *field_key);

/** Access structured entry count.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of structured entries.
 */
int gcal_contact_get_structured_address_count(gcal_contact_t contact);

/** Access structured address count object.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field.
 */
int *gcal_contact_get_structured_address_count_obj(gcal_contact_t contact);

/** Access structured entry type.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param structured_entry_nr The number of the specific entry.
 *
 * @param structured_entry_count Number of all entries.
 *
 * @return Type of entry
 */
gcal_address_type gcal_contact_get_structured_address_type(gcal_contact_t contact, int structured_entry_nr, int structured_entry_count);

/** Access structured address type object.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field
 */
char ***gcal_contact_get_structured_address_type_obj(gcal_contact_t contact);

/** Access preferred structured address number.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of preferred structured address.
 *
 */
int gcal_contact_get_pref_structured_address(gcal_contact_t contact);

/** Access Google group membership info.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_groupMembership(gcal_contact_t contact, int i);

/** Access Google group membership info count.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Number of group membership entries.
 */
int gcal_contact_get_groupMembership_count(gcal_contact_t contact);

/** Access contact birthday.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_birthday(gcal_contact_t contact);

/** Access contact photo data.
 *
 * When no year is set in Google, Year is set to 1900 in KABC
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return Pointer to internal object field (dont free it!) or NULL (in error
 * case or if the field is not set). If the entry hasn't this field in the
 * atom stream, it will be set to an empty string (i.e. "").
 */
char *gcal_contact_get_photo(gcal_contact_t contact);

/** Access contact photo data length.
 *
 * It can required to known where the data ends (since is a binary blob
 * you cannot use strlen).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 0 for no photo, 1 for photo link existence and > 1 when having the
 * photo data. -1 for error case.
 */
unsigned int gcal_contact_get_photolength(gcal_contact_t contact);

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
int gcal_contact_set_title(gcal_contact_t contact, const char *field);

/** Sets contact email.
 *
 * This field is a hard requirement to create a new contact. Google server
 * supports more e-mails with special tags too.
 *
 * If you need to add a contact entry with all optional fields, an
 * alternative is to use \ref gcal_add_xmlentry.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Email address.
 *
 * @param type Email address type.
 *
 * @param pref Preferred email address (0=no, 1=yes).
 *
 * @return 0 for success, -1 otherwise
 */

int gcal_contact_add_email_address(gcal_contact_t contact, const char *field,
				   gcal_email_type type, int pref);

/** Sets the prefered email.
 *
 * This is a convenience function, it is implemented internally using
 * \ref gcal_contact_add_email_address.
 *
 * The email type defauls to HOME.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param pref_email Email address.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_email(gcal_contact_t contact, const char *pref_email);

/** Set contact email label.
 *
 * @param i Number of the email contact.
 *
 * @param label Label for the email contact.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_email_label(gcal_contact_t contact, int i, const char *label);

/* TODO: document new functions */
int gcal_contact_delete_email_addresses(gcal_contact_t contact);


/** Sets contact edit url.
 *
 * This field is a hard requirement to edit/delete a contact. Starting with
 * google data API 2.0, the ETag is also required.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field String with the edit url email
 * (e.g. "http://www.google.com/m8/feeds/contacts/user%40gmail.com/base/2").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_contact_set_url(gcal_contact_t contact, const char *field);


/** Sets contact ID.
 *
 * Each contact has an ID (but this can extracted from the edit_url).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field String with contact ID (e.g. "joe.doe@nobody.com").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_contact_set_id(gcal_contact_t contact, const char *field);


/** Sets contact ETag.
 *
 * Starting with google data API 2.0, the ETag is used for versioning the
 * entries. Is required for edit/delete.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field String with contact ETag (e.g. "Q3c5eDVSLyp7ImA9WxRbFE0KRAY.").
 *
 * @return 0 for sucess, -1 otherwise.
 */
int gcal_contact_set_etag(gcal_contact_t contact, const char *field);



/** Sets the contact telephone.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Phone number.
 *
 * @param type Phone number type.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_add_phone_number(gcal_contact_t contact, const char *field,
				  gcal_phone_type type);


/** Sets contact prefered phone.
 *
 * This is a convenience function, it is implemented internally using
 * \ref gcal_contact_add_phone_number. Pay attention that when this
 * function is called, it will destroy all the contact's phone list
 * to ensure that the prefered email will be the first to be added.
 *
 * The phone type default to P_MOBILE.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param phone A phone number (e.g. "+551132314412")
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_phone(gcal_contact_t contact, const char *phone);

/** Set contact phone label.
 *
 * @param i Number of the phone contact.
 *
 * @param label Label for the phone contact.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_phone_number_label(gcal_contact_t contact, int i, const char *label);

/* TODO: document new functions */
int gcal_contact_delete_phone_numbers(gcal_contact_t contact);

/** Sets contact IM.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param protocol IM protocol.
 *
 * @param address IM address.
 *
 * @param type IM type.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_add_im(gcal_contact_t contact, const char *protocol,
			const char *address, gcal_im_type type, int pref);

/** Set contact IM label.
 *
 * @param i Number of the IM contact.
 *
 * @param label Label for the IM contact.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_im_label(gcal_contact_t contact, int i, const char *label);

/** Deletes contact IM.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_delete_im(gcal_contact_t contact);

/** Sets the contact address (Google API v2.0 gd:postalAddress).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Address string.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_address(gcal_contact_t contact, const char *field);

/** Sets the contact full address number (structuredPostalAddress).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param type Address type.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_structured_address_nr(gcal_contact_t contact,
					   gcal_address_type type);

/** Sets the contact preferred address number (structuredPostalAddress).
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param pref_address Number of preferred address.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_pref_structured_address(gcal_contact_t contact, int pref_address);

/** Sets a structured entry.
 *
 * @param structured_entry A structured entry object, see \ref gcal_structured_subvalues.
 *
 * @param structured_entry_nr Index of the entry.
 *
 * @param structured_entry_count Number of all entries.
 *
 * @param field_key Key of the structured entry.
 *
 * @param field_value Corresponding value.
 *
 * @param structured_entry_type Structured entry type.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_structured_entry(gcal_structured_subvalues_t structured_entry,
				      int structured_entry_nr,
				      int structured_entry_count,
				      const char *field_key,
				      const char *field_value );

/** Deletes a structured entry.
 *
 * @param structured_entry A structured entry object, see \ref gcal_structured_subvalues.
 *
 * @param *structured_entry_count Pointer to the structured entry count object.
 *
 * @param ***structured_entry_type Pointer to the structured entry type object.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_delete_structured_entry(gcal_structured_subvalues_t structured_entry,
					 int *structured_entry_count,
					 char ***structured_entry_type);

/** Sets the contact group membership info.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Group name.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_add_groupMembership(gcal_contact_t contact, char *field);

/* TODO: document new functions */
int gcal_contact_delete_groupMembership(gcal_contact_t contact);


/** Sets the contact birthday
 *
 * When no year is set in Google, Year is set to 1900 in KABC
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field birthday string.
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_birthday(gcal_contact_t contact, const char *field);

/** Sets the organization title.
 *
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Organization title string (i.e. "C++ programmer").
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_profission(gcal_contact_t contact, const char *field);

/** Sets the organization name.
 *
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field Organization name string (i.e. "Foo Software Inc.").
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_organization(gcal_contact_t contact, const char *field);

/** Sets the occupation/profession.
 *
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field occupation/profession name string (i.e. "Carpenter").
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_occupation(gcal_contact_t contact, const char *field);

/** Sets contact description.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field A note, description (i.e. "Really funny guy")
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_content(gcal_contact_t contact, const char *field);

/** Sets contact nickname.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field The nickname
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_nickname(gcal_contact_t contact, const char *field);

/** Sets contact website.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field URL
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_homepage(gcal_contact_t contact, const char *field);

/** Sets contact blog.
 *
 * @param contact A contact object, see \ref gcal_contact.
 *
 * @param field URL
 *
 * @return 0 for success, -1 otherwise
 */
int gcal_contact_set_blog(gcal_contact_t contact, const char *field);

/* TODO: document new functions */
int gcal_contact_set_photo(gcal_contact_t contact, const char *field,
			   int length);


#endif
