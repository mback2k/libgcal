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


/** Strings associated with phone number types */
const char* gcal_phone_type_str[] = {
	"assistant",		// P_ASSISTANT
	"callback",		// P_CALLBACK
	"car",			// P_CAR
	"company_main",		// P_COMPANY_MAIN
	"fax",			// P_FAX
	"home",			// P_HOME
	"home_fax",		// P_HOME_FAX
	"isdn",			// P_ISDN
	"main",			// P_MAIN
	"mobile",		// P_MOBILE
	"other",		// P_OTHER
	"other_fax",		// P_OTHER_FAX
	"pager",		// P_PAGER
	"radio",		// P_RADIO
	"telex",		// P_TELEX
	"tty_tdd",		// P_TTY_TDD
	"work",			// P_WORK
	"work_fax",		// P_WORK_FAX
	"work_mobile",		// P_WORK_MOBILE
	"work_pager"		// P_WORK_PAGER
};

/** Strings associated with email types */
const char* gcal_email_type_str[] = {
	"home",				// E_HOME
	"other",				// E_OTHER
	"work"				// E_WORK
};

gcal_contact_t gcal_contact_new(char *raw_xml)
{
	gcal_contact_t contact = NULL;
	dom_document *doc;
	int result = -1;

	contact = (gcal_contact_t) malloc(sizeof(struct gcal_contact));
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

void gcal_contact_delete(gcal_contact_t contact)
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


int gcal_add_contact(gcal_t gcalobj, gcal_contact_t contact)
{
	int result = -1;
	struct gcal_contact updated;
	gcal_init_contact(&updated);

	if ((!gcalobj) || (!contact))
		goto exit;


	result = gcal_create_contact(gcalobj, contact, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: id, updated, edit_uri, etag, photo url  */
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

	if (contact->photo)
		free(contact->photo);
	contact->photo = updated.photo;
	updated.photo = NULL;

	/* Cleanup updated contact */
	gcal_destroy_contact(&updated);

exit:
	return result;
}

int gcal_update_contact(gcal_t gcalobj, gcal_contact_t contact)
{
	int result = -1;
	struct gcal_contact updated;
	gcal_init_contact(&updated);

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

	if (contact->photo)
		free(contact->photo);
	contact->photo = updated.photo;
	updated.photo = NULL;

	/* Cleanup updated contact */
	gcal_destroy_contact(&updated);

exit:
	return result;
}

int gcal_erase_contact(gcal_t gcalobj, gcal_contact_t contact)
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

gcal_contact_t gcal_contact_element(struct gcal_contact_array *contacts,
				    size_t _index)

{
	struct gcal_contact *contact = NULL;
	if ((!contacts) || (_index > (contacts->length - 1)) ||
	    (contacts->length == 0))
		return contact;

	contact = &contacts->entries[_index];
	return contact;
}

char *gcal_contact_get_xml(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_xml(&(contact->common));
}

char *gcal_contact_get_id(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_id(&(contact->common));
}

char *gcal_contact_get_updated(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_updated(&(contact->common));
}

char *gcal_contact_get_title(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_title(&(contact->common));
}

char *gcal_contact_get_url(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_url(&(contact->common));
}

char *gcal_contact_get_etag(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return gcal_get_etag(&(contact->common));
}

char gcal_contact_is_deleted(gcal_contact_t contact)
{
	if ((!contact))
		return -1;
	return gcal_get_deleted(&(contact->common));
}


/* This are the fields unique to calendar contacts */
int gcal_contact_get_emails_count(gcal_contact_t contact)
{
	if ((!contact))
		return -1;
	return contact->emails_nr;
}

int gcal_contact_get_pref_email(gcal_contact_t contact)
{
	if ((!contact))
		return -1;
	return contact->pref_email;
}

char *gcal_contact_get_email_address(gcal_contact_t contact, int i)
{
	if ((!contact))
		return NULL;
	if (!(contact->emails_field) || (i >= contact->emails_nr))
		return NULL;
	return contact->emails_field[i];
}

gcal_email_type gcal_contact_get_email_address_type(gcal_contact_t contact, int i)
{
	gcal_email_type result = E_INVALID;
	int j;
	
	if ((!contact))
		return result;
	if (!(contact->emails_type) || (i >= contact->emails_nr))
		return result;
	for (j = 0; j < E_ITEMS_COUNT; j++)
		if (!strcmp(contact->emails_type[i], gcal_email_type_str[j]))
			result = j;
	return result;
}

char *gcal_contact_get_content(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return contact->content;
}

char *gcal_contact_get_organization(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return contact->org_name;
}

char *gcal_contact_get_profission(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return contact->org_title;
}

char *gcal_contact_get_im(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return contact->im;
}

int gcal_contact_get_phone_numbers_count(gcal_contact_t contact)
{
	if ((!contact))
		return -1;
	return contact->phone_numbers_nr;
}

char *gcal_contact_get_phone_number(gcal_contact_t contact, int i)
{
	if ((!contact))
		return NULL;
	if (!(contact->phone_numbers_field) || (i >= contact->phone_numbers_nr))
		return NULL;
	return contact->phone_numbers_field[i];
}

gcal_phone_type gcal_contact_get_phone_number_type(gcal_contact_t contact, int i)
{
	gcal_phone_type result = P_INVALID;
	int j;
	
	if ((!contact))
		return result;
	if (!(contact->phone_numbers_type) || (i >= contact->phone_numbers_nr))
		return result;
	for (j = 0; j < P_ITEMS_COUNT; j++)
		if (!strcmp(contact->phone_numbers_type[i], gcal_phone_type_str[j]))
			result = j;
	return result;
}

char *gcal_contact_get_address(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;
	return contact->post_address;
}

int gcal_contact_get_groupMembership_count(gcal_contact_t contact)
{
	if ((!contact))
		return -1;
	return contact->groupMembership_nr;
}

char *gcal_contact_get_groupMembership(gcal_contact_t contact, int i)
{
	if ((!contact))
		return NULL;
	if (!(contact->groupMembership) || (i >= contact->groupMembership_nr))
		return NULL;
	return contact->groupMembership[i];
}

char *gcal_contact_get_photo(gcal_contact_t contact)
{
	if ((!contact))
		return NULL;

	return contact->photo_data;
}

unsigned int gcal_contact_get_photolength(gcal_contact_t contact)
{
	if ((!contact))
		return -1;

	return contact->photo_length;
}

/* Here starts the gcal_contact setters */
int gcal_contact_set_title(gcal_contact_t contact, const char *field)
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

int gcal_contact_delete_email_addresses(gcal_contact_t contact)
{
	int result = -1;
	int temp;

	if (!contact)
		return result;

	if (contact->emails_nr > 0) {
		for (temp = 0; temp < contact->emails_nr; temp++) {
			if (contact->emails_field[temp])
				free(contact->emails_field[temp]);
			if (contact->emails_type[temp])
				free(contact->emails_type[temp]);
		}
		free(contact->emails_field);
		free(contact->emails_type);
	}

	contact->emails_nr = contact->pref_email = 0;
	contact->emails_field = contact->emails_type = 0;

	result = 0;

	return result;
}

int gcal_contact_add_email_address(gcal_contact_t contact, const char *field, gcal_email_type type, int pref)
{
	int result = -1;

	if ((!contact) || (!field) || (type<0) || (type>=E_ITEMS_COUNT))
		return result;

	contact->emails_field = (char**) realloc(contact->emails_field, (contact->emails_nr+1) * sizeof(char*));
	contact->emails_field[contact->emails_nr] = strdup(field);

	contact->emails_type = (char**) realloc(contact->emails_type, (contact->emails_nr+1) * sizeof(char*));
	contact->emails_type[contact->emails_nr] = strdup(gcal_email_type_str[type]);

	if (pref)
		contact->pref_email = contact->emails_nr;

	contact->emails_nr++;

	result = 0;

	return result;
}

int gcal_contact_set_url(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->common.edit_uri)
		free(contact->common.edit_uri);

	contact->common.edit_uri = strdup(field);
	if (contact->common.edit_uri)
		result = 0;

	return result;
}


int gcal_contact_set_id(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->common.id)
		free(contact->common.id);

	contact->common.id = strdup(field);
	if (contact->common.id)
		result = 0;

	return result;
}


int gcal_contact_set_etag(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->common.etag)
		free(contact->common.etag);

	contact->common.etag = strdup(field);
	if (contact->common.etag)
		result = 0;

	return result;
}

int gcal_contact_delete_phone_numbers(gcal_contact_t contact)
{
	int result = -1;
	int temp;

	if (!contact)
		return result;

	if (contact->phone_numbers_nr > 0) {
		for (temp = 0; temp < contact->phone_numbers_nr; temp++) {
			if (contact->phone_numbers_field[temp])
				free(contact->phone_numbers_field[temp]);
			if (contact->phone_numbers_type[temp])
				free(contact->phone_numbers_type[temp]);
		}
		free(contact->phone_numbers_field);
		free(contact->phone_numbers_type);
	}

	contact->phone_numbers_nr = 0;

	result = 0;

	return result;
}

int gcal_contact_add_phone_number(gcal_contact_t contact, const char *field, gcal_phone_type type)
{
	int result = -1;

	if ((!contact) || (!field) || (type<0) || (type>=P_ITEMS_COUNT))
		return result;

	contact->phone_numbers_field = (char**) realloc(contact->phone_numbers_field, (contact->phone_numbers_nr+1) * sizeof(char*));
	contact->phone_numbers_field[contact->phone_numbers_nr] = strdup(field);

	contact->phone_numbers_type = (char**) realloc(contact->phone_numbers_type, (contact->phone_numbers_nr+1) * sizeof(char*));
	contact->phone_numbers_type[contact->phone_numbers_nr] = strdup(gcal_phone_type_str[type]);

	contact->phone_numbers_nr++;

	result = 0;

	return result;
}

int gcal_contact_set_address(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->post_address)
		free(contact->post_address);

	contact->post_address = strdup(field);
	if (contact->post_address)
		result = 0;

	return result;
}

int gcal_contact_delete_groupMembership(gcal_contact_t contact)
{
	int result = -1;
	int temp;

	if (!contact)
		return result;

	if (contact->groupMembership_nr > 0) {
		for (temp = 0; temp < contact->groupMembership_nr; temp++) {
			if (contact->groupMembership[temp])
				free(contact->groupMembership[temp]);
		}
		free(contact->groupMembership);
	}

	contact->groupMembership_nr = 0;

	result = 0;

	return result;
}

int gcal_contact_add_groupMembership(gcal_contact_t contact, char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	contact->groupMembership = (char**) realloc(contact->groupMembership, (contact->groupMembership_nr+1) * sizeof(char*));
	contact->groupMembership[contact->groupMembership_nr] = strdup(field);

	contact->groupMembership_nr++;

	result = 0;

	return result;
}

int gcal_contact_set_profission(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->org_name)
		free(contact->org_name);

	contact->org_name = strdup(field);
	if (contact->org_name)
		result = 0;

	return result;

}

int gcal_contact_set_organization(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->org_title)
		free(contact->org_title);

	contact->org_title = strdup(field);
	if (contact->org_title)
		result = 0;

	return result;
}

int gcal_contact_set_content(gcal_contact_t contact, const char *field)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->content)
		free(contact->content);

	contact->content = strdup(field);
	if (contact->content)
		result = 0;

	return result;

}

int gcal_contact_set_photo(gcal_contact_t contact, const char *field,
			   int length)
{
	int result = -1;

	if ((!contact) || (!field))
		return result;

	if (contact->photo_data)
		if (contact->photo_length > 1)
			free(contact->photo_data);

	if (!(contact->photo_data = malloc(length * sizeof(unsigned char))))
		return result;

	memcpy(contact->photo_data, field, length);
	contact->photo_length = length;
	result = 0;

	return result;
}
