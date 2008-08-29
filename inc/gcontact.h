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

struct gcal_contact_array {
	/** See \ref gcal_contact. */
	struct gcal_contact *entries;
	/** The number of entries */
	size_t length;
};


/** Creates a new google contact object.
 *
 * If you are going to add new event, see also \ref gcal_add_contact.
 *
 * @return A gcal_contact object on success or NULL otherwise.
 */
gcal_contact gcal_contact_new(void);

/** Free an gcal contact object.
 *
 *
 * @param event An gcal event object, see also \ref gcal_event_new.
 */
void gcal_contact_delete(gcal_contact contact);


/** Helper function, does all contact events dump and parsing, returning
 * the data as an array of \ref gcal_contact.
 *
 * @param gcalobj A libgcal object, must be previously authenticated with
 * \ref gcal_get_authentication.
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
 * @param events A pointer to an contacts array structure. See
 * \ref gcal_contact_array.
 */
void gcal_cleanup_contacts(struct gcal_contact_array *contacts);



gcal_contact gcal_contact_element(struct gcal_contact_array *contacts,
				  size_t _index);


int gcal_add_contact(gcal_t gcalobj, gcal_contact contact);
int gcal_update_contact(gcal_t gcalobj, gcal_contact contact);
int gcal_erase_contact(gcal_t gcalobj, gcal_contact contact);
int gcal_get_updated_contacts(gcal_t gcal_obj,
			      struct gcal_contact_array *contacts,
			      char *timestamp);


/* Here starts gcal_contact accessors */
char *gcal_contact_get_id(gcal_contact contact);
char *gcal_contact_get_updated(gcal_contact contact);
char *gcal_contact_get_title(gcal_contact contact);
char *gcal_contact_get_url(gcal_contact contact);
char *gcal_contact_get_xml(gcal_contact contact);
char gcal_contact_is_deleted(gcal_contact contact);


/* This are the fields unique to contacts */
char *gcal_contact_get_email(gcal_contact contact);
char *gcal_contact_get_content(gcal_contact contact);
char *gcal_contact_get_orgname(gcal_contact contact);
char *gcal_contact_get_orgtitle(gcal_contact contact);
char *gcal_contact_get_im(gcal_contact contact);
char *gcal_contact_get_phone(gcal_contact contact);
char *gcal_contact_get_address(gcal_contact contact);


/* Here starts the gcal_contact setters */
int gcal_contact_set_title(gcal_contact contact, char *field);
int gcal_contact_set_email(gcal_contact contact, char *field);

/* TODO: Contacts extra fields, not implemented in internal functions
 * see ticket: http://code.google.com/p/libgcal/issues/detail?id=4
 */
int gcal_contact_set_phone(gcal_contact contact, char *field);


#endif
