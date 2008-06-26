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
#include "internal_gcal.h"

gcal_contact gcal_contact_new(void)
{
	gcal_contact result = NULL;
	result = (gcal_contact) malloc(sizeof(struct gcal_contact));
	if (!result)
		return result;

	gcal_init_contact(result);
	return result;

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

	if ((!gcalobj) || (!contact_array))
		return result;

	result = gcal_dump(gcalobj);
	if (result == -1)
		return result;

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
