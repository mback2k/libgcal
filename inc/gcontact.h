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
 * @file   gcont.h
 * @author Adenilson Cavalcanti
 * @date   Fri May 30 14:52:52 2008
 *
 * @brief  Google contacts libgcal header file.
 *
 * Public functions and data structures for contacts.
 *
 */

#ifndef __GCONTACT_LIB__
#define __GCONTACT_LIB__

#include <stdlib.h>
#include "gcal.h"

/** Contact data type */
struct gcal_contact {
	/** element ID */
	char *id;
	/** Time when the event was updated. */
	char *updated;
	/** The contact name */
	char *title;
	/** The edit URL */
	char *edit_uri;
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

/** Extracts from the atom stream the contact entries  (you should
 * had got the atom stream before, using \ref gcal.h:gcal_dump).
 *
 * Pay attention that it returns a vector of structures that must be destroyed
 * using \ref gcal_destroy_contacts.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param length Pointer to an unsigned int, it will have the vector length.
 *
 * @return A pointer on sucess, NULL otherwise.
 */
struct gcal_contact *gcal_get_contacts(struct gcal_resource *ptr_gcal,
				       size_t *length);


/** Cleanup memory of 1 contact structure pointer.
 *
 *
 * @param entry A pointer to a \ref gcal_contact.
 */
void gcal_destroy_contact(struct gcal_contact *contact);



/** Cleanup the memory of a vector of calendar entries created using
 * \ref gcal_get_entries.
 *
 * @param entries A pointer to a vector of \ref gcal_entries structure.
 *
 * @param length The vector length.
 */
void gcal_destroy_contacts(struct gcal_contact *contacts, size_t length);


#endif
