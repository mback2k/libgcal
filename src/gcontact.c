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
 * @author Adenilson Cavalcanti
 * @date   Fri May 30 15:30:35 2008
 *
 * @brief  Base file for google contacts service access library.
 *
 * \todo:
 * - move HTTP code common to gcalendar to a distinct module
 * - implement contact add/delete/edit
 *
 */
#include "internal_gcal.h"
#include "gcontact.h"
#include "gcal_parser.h"


struct gcal_contact *gcal_get_contacts(struct gcal_resource *ptr_gcal,
				       size_t *length)

{
	int result = -1;
	struct gcal_contact *ptr_res = NULL;

	if (!ptr_gcal)
		goto exit;

	if (!ptr_gcal->buffer || !ptr_gcal->has_xml)
		goto exit;

	ptr_gcal->document = build_dom_document(ptr_gcal->buffer);
	if (!ptr_gcal->document)
		goto exit;


	result = get_entries_number(ptr_gcal->document);
	if (result == -1)
		goto cleanup;

	ptr_res = malloc(sizeof(struct gcal_contact) * result);
	if (!ptr_res)
		goto cleanup;

	*length = result;
	result = extract_all_contacts(ptr_gcal->document, ptr_res, result);
	if (result == -1) {
		free(ptr_res);
		ptr_res = NULL;
	}

	goto exit;

cleanup:
	clean_dom_document(ptr_gcal->document);
	ptr_gcal->document = NULL;

exit:

	return ptr_res;

}

static void clean_string(char *ptr_str)
{
	if (ptr_str)
		free(ptr_str);
}

void gcal_destroy_contact(struct gcal_contact *contact)
{
	if (!contact)
		return;

	clean_string(contact->id);
	clean_string(contact->updated);
	clean_string(contact->title);
	clean_string(contact->edit_uri);
	clean_string(contact->email);

	/* Extra fields */
	clean_string(contact->content);
	clean_string(contact->org_name);
	clean_string(contact->org_title);
	clean_string(contact->im);
	clean_string(contact->phone_number);
	clean_string(contact->post_address);

}

void gcal_destroy_contacts(struct gcal_contact *contacts, size_t length)
{

	size_t i = 0;
	if (!contacts)
		return;

	for (; i < length; ++i)
		gcal_destroy_contact((contacts + i));

	free(contacts);
}

int gcal_create_contact(struct gcal_contact *contact,
			struct gcal_resource *ptr_gcal)
{
	int result = -1;
	(void)contact;
	(void)ptr_gcal;


	return result;
}
