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
 * @file   gcontact.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Thu Jun 26 07:37:03 2008
 *
 * @brief  libgcal google contacts user public API.
 *
 * Use this functions to handle common tasks when dealing with google contacts.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include "gcontact.h"
#include "gcal_parser.h"
#include "internal_gcal.h"

gcal_contact gcal_contact_new(char *raw_xml)
{
	gcal_contact contact = NULL;
	dom_document *doc;
	int result = -1;

	contact = (gcal_contact) malloc(sizeof(struct gcal_contact));
	if (!contact)
		goto exit;

	gcal_init_contact(contact);
	if (!raw_xml)
		goto exit;

	/* Builds a doc, parse and init object */
	doc = build_dom_document(raw_xml);
	if (!doc)
		goto cleanup;

	result = extract_all_contacts(doc, contact, 1);
	clean_dom_document(doc);

cleanup:
	if (result) {
		free(contact);
		contact = NULL;
	}
exit:
	return contact;
}

void gcal_contact_delete(gcal_contact contact)
{
	if (!contact)
		return;

	gcal_destroy_contact(contact);
	free(contact);

}

int gcal_get_contacts(gcal_t gcalobj, struct gcal_contact_array *contact_array)
{
	int result = -1;
	if (contact_array)
		contact_array->length = 0;

	if ((!gcalobj) || (!contact_array))
		return result;

	result = gcal_dump(gcalobj);
	if (result == -1) {
		contact_array->entries = NULL;
		contact_array->length = 0;
		return result;
	}

	contact_array->entries = gcal_get_all_contacts(gcalobj,
						       &contact_array->length);
	if (!contact_array->entries)
		return result;

	result = 0;

	return result;

}

void gcal_cleanup_contacts(struct gcal_contact_array *contacts)
{
	if (!contacts)
		return;

	gcal_destroy_contacts(contacts->entries, contacts->length);
	contacts->length = 0;
	contacts->entries = NULL;

}


int gcal_add_contact(gcal_t gcalobj, gcal_contact contact)
{
	int result = -1;
	struct gcal_contact updated;
	gcal_init_contact(&updated);

	if ((!gcalobj) || (!contact))
		goto exit;


	result = gcal_create_contact(gcalobj, contact, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: id, updated, edit_uri, etag  */
	if (contact->common.id)
		free(contact->common.id);
	contact->common.id = updated.common.id;
	updated.common.id = NULL;

	if (contact->common.updated)
		free(contact->common.updated);
	contact->common.updated = updated.common.updated;
	updated.common.updated = NULL;

	if (contact->common.edit_uri)
		free(contact->common.edit_uri);
	contact->common.edit_uri = updated.common.edit_uri;
	updated.common.edit_uri = NULL;

	if (contact->common.etag)
		free(contact->common.etag);
	contact->common.etag = updated.common.etag;
	updated.common.etag = NULL;

	/* Cleanup updated contact */
	gcal_destroy_contact(&updated);

exit:
	return result;
}

int gcal_update_contact(gcal_t gcalobj, gcal_contact contact)
{
	int result = -1;
	struct gcal_contact updated;

	if ((!gcalobj) || (!contact))
		goto exit;


	result = gcal_edit_contact(gcalobj, contact, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: updated, edit_uri, etag */
	if (contact->common.updated)
		free(contact->common.updated);
	contact->common.updated = updated.common.updated;
	updated.common.updated = NULL;

	if (contact->common.edit_uri)
		free(contact->common.edit_uri);
	contact->common.edit_uri = updated.common.edit_uri;
	updated.common.edit_uri = NULL;

	if (contact->common.etag)
		free(contact->common.etag);
	contact->common.etag = updated.common.etag;
	updated.common.etag = NULL;

	/* Cleanup updated contact */
	gcal_destroy_contact(&updated);

exit:
	return result;
}

int gcal_erase_contact(gcal_t gcalobj, gcal_contact contact)
{
	int result = -1;
	if ((!gcalobj) || (!contact))
		goto exit;

	result = gcal_delete_contact(gcalobj, contact);
exit:
	return result;
}

int gcal_get_updated_contacts(gcal_t gcal_obj,
			      struct gcal_contact_array *contacts,
			      char *timestamp)
{
	int result = -1;
	if (contacts)
		contacts->length = 0;

	if ((!gcal_obj) || (!contacts))
		return result;

	result = gcal_query_updated(gcal_obj, timestamp);
	if (result) {
		contacts->entries = NULL;
		contacts->length = 0;
		return result;
	}

	contacts->entries = gcal_get_all_contacts(gcal_obj, &contacts->length);
	if (contacts->entries)
		result = 0;

	return result;
}

gcal_contact gcal_contact_element(struct gcal_contact_array *contacts,
				  size_t _index)

{
	struct gcal_contact *contact = NULL;
	if ((!contacts) || (_index > (contacts->length - 1)) ||
	    (contacts->length == 0))
		return contact;

	contact = &contacts->entries[_index];
	return contact;
}

char *gcal_contact_get_xml(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_xml(&(contact->common));
}

char *gcal_contact_get_id(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_id(&(contact->common));
}

char *gcal_contact_get_updated(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_updated(&(contact->common));
}

char *gcal_contact_get_title(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_title(&(contact->common));
}

char *gcal_contact_get_url(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_url(&(contact->common));
}

char gcal_contact_is_deleted(gcal_contact contact)
{
	if ((!contact))
		return -1;
	return gcal_get_deleted(&(contact->common));
}


/* This are the fields unique to calendar contacts */
char *gcal_contact_get_email(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->email;
}

char *gcal_contact_get_content(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->content;
}

char *gcal_contact_get_orgname(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->org_name;
}

char *gcal_contact_get_orgtitle(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->org_title;
}

char *gcal_contact_get_im(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->im;
}

char *gcal_contact_get_phone(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->phone_number;
}

char *gcal_contact_get_address(gcal_contact contact)
{
	if ((!contact))
		return NULL;
	return contact->post_address;
}

/* Here starts the gcal_contact setters */
int gcal_contact_set_title(gcal_contact contact, char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->common.title)
		free(contact->common.title);

	contact->common.title = strdup(field);
	if (contact->common.title)
		result = 0;

	return result;
}

int gcal_contact_set_email(gcal_contact contact, char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->email)
		free(contact->email);

	contact->email = strdup(field);
	if (contact->email)
		result = 0;

	return result;
}

/* TODO: Contacts extra fields, not implemented in internal functions
 * see ticket: http://code.google.com/p/libgcal/issues/detail?id=4
 */
int gcal_contact_set_phone(gcal_contact contact, char *field)
{
	(void)contact;
	(void)field;
	return -1;
}
