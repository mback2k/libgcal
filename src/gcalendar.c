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
 * @file   gcalendar.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Tue Jun 24 16:17:25 2008
 *
 * @brief  libgcal calendar user public API.
 *
 * Use this functions to handle common tasks when dealing with google calendar.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include "gcalendar.h"
#include "internal_gcal.h"
#include "gcal_parser.h"
#include "msvc_hacks.h"

gcal_t gcal_new(gservice mode)
{
	return gcal_construct(mode);
}

void gcal_delete(gcal_t gcal_obj)
{
	gcal_destroy(gcal_obj);
}

gcal_event_t gcal_event_new(char *raw_xml)
{
	gcal_event_t event = NULL;
	dom_document *doc;
	int result = -1;

	event = malloc(sizeof(struct gcal_event));
	if (!event)
		goto exit;
	gcal_init_event(event);
	if (!raw_xml)
		goto exit;

	/* Builds a doc, parse and init object */
	doc = build_dom_document(raw_xml);
	if (!doc)
		goto cleanup;

	result = extract_all_entries(doc, event, 1);
	clean_dom_document(doc);

cleanup:
	if (result) {
		free(event);
		event = NULL;
	}

exit:
	return event;
}

void gcal_event_delete(gcal_event_t event)
{

	if (!event)
		return;

	gcal_destroy_entry(event);
	free(event);
}

int gcal_get_edit_url(char *entry, char **extracted_url)
{
	int result = -1;
	if (!entry)
		goto exit;

	result = get_edit_url(entry, strlen(entry), extracted_url);

exit:
	return result;

}

int gcal_get_extract_etag(char *entry, char **extracted_etag)
{
	int result = -1;
	if (!entry)
		goto exit;

	result = get_edit_etag(entry, strlen(entry), extracted_etag);

exit:
	return result;

}

int gcal_add_xmlentry(gcal_t gcal_obj, char *xml_entry, char **xml_updated)
{
	int result = -1, length = 0;
	char *buffer = NULL;

	if ((!gcal_obj) || (!xml_entry))
		goto exit;

	if (!(strcmp(gcal_obj->service, "cl")))
		result = up_entry(xml_entry, strlen(xml_entry), gcal_obj,
				  GCAL_EDIT_URL, NULL,
				  POST, NULL, GCAL_EDIT_ANSWER);

	else {
		/* Mounts URL */
		length = sizeof(GCONTACT_START) + sizeof(GCONTACT_END) +
			strlen(gcal_obj->user) + sizeof(GCAL_DELIMITER) +
			strlen(gcal_obj->domain) + 1;
		buffer = (char *) malloc(length);
		if (!buffer)
			goto cleanup;
		snprintf(buffer, length - 1, "%s%s%s%s%s", GCONTACT_START,
			 gcal_obj->user, GCAL_DELIMITER, gcal_obj->domain,
			 GCONTACT_END);

		result = up_entry(xml_entry, strlen(xml_entry), gcal_obj,
				  buffer, NULL,
				  POST, NULL, GCAL_EDIT_ANSWER);
	}

	if (!result)
		if (xml_updated)
			*xml_updated = strdup(gcal_obj->buffer);

cleanup:
	if (buffer)
		free(buffer);

exit:
	return result;
}

int gcal_update_xmlentry(gcal_t gcal_obj, char *xml_entry, char **xml_updated,
			 char *edit_url, char *etag)
{
	char *url = NULL, *pvt_etag = NULL;
	int result = -1;
	char buffer[512];
	const char if_match[] = "If-Match: ";

	memset(buffer, '\0', sizeof(buffer));

	if ((!gcal_obj) || (!xml_entry))
		goto exit;

	if (!edit_url) {
		result = get_edit_url(xml_entry, strlen(xml_entry), &url);
		if (result)
			goto exit;

	} else
		if (!(url = strdup(edit_url)))
			goto exit;

	if (!etag) {
		if ((result = get_edit_etag(xml_entry, strlen(xml_entry),
					    &pvt_etag)))
			goto exit;
		else
			etag = pvt_etag;
	}

	/* Mounts costum HTTP header using ETag */
	snprintf(buffer, sizeof(buffer) - 1, "%s\%s",
		 if_match, etag);

	result = up_entry(xml_entry, strlen(xml_entry), gcal_obj, url, buffer,
			  PUT, NULL, GCAL_DEFAULT_ANSWER);

	if (!result)
		if (xml_updated)
			*xml_updated = strdup(gcal_obj->buffer);

	if (url)
		free(url);

	if (pvt_etag)
		free(pvt_etag);
exit:

	return result;
}

int gcal_erase_xmlentry(gcal_t gcal_obj, char *xml_entry)
{
	char *edit_url = NULL;
	int result = -1;
	/* I could use just 1 structure object and benefit from the fact
	 * that both have the same field type/name alignment (that would
	 * save 1 structure from the stack). But it would break with any
	 * change in the field type/alignment, I don't think its worthwhile.
	 */
	struct gcal_event event;
	struct gcal_contact contact;

	if ((!gcal_obj) || (!xml_entry))
		goto exit;

	result = get_edit_url(xml_entry, strlen(xml_entry), &edit_url);
	if (result)
		goto exit;
	event.common.edit_uri = edit_url;
	contact.common.edit_uri = edit_url;

	if (!(strcmp(gcal_obj->service, "cl")))
		result = gcal_delete_event(gcal_obj, &event);

	else
		result = gcal_delete_contact(gcal_obj, &contact);

	if (edit_url)
		free(edit_url);

exit:
	return result;
}


int gcal_add_event(gcal_t gcal_obj, gcal_event_t event)
{
	int result = -1;
	struct gcal_event updated;
	gcal_init_event(&updated);

	if ((!gcal_obj) || (!event))
		goto exit;

	result = gcal_create_event(gcal_obj, event, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: id, updated, edit_uri, etag  */
	if (event->common.id)
		free(event->common.id);
	event->common.id = updated.common.id;
	updated.common.id = NULL;

	if (event->common.updated)
		free(event->common.updated);
	event->common.updated = updated.common.updated;
	updated.common.updated = NULL;

	if (event->common.edit_uri)
		free(event->common.edit_uri);
	event->common.edit_uri = updated.common.edit_uri;
	updated.common.edit_uri = NULL;

	if (event->common.etag)
		free(event->common.etag);
	event->common.etag = updated.common.etag;
	updated.common.etag = NULL;

	/* Cleanup updated event */
	gcal_destroy_entry(&updated);
exit:
	return result;
}

int gcal_update_event(gcal_t gcal_obj, gcal_event_t event)
{
	int result = -1;
	struct gcal_event updated;

	if ((!gcal_obj) || (!event))
		goto exit;

	result = gcal_edit_event(gcal_obj, event, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: updated, edit_uri, etag */
	if (event->common.updated)
		free(event->common.updated);
	event->common.updated = updated.common.updated;
	updated.common.updated = NULL;

	if (event->common.edit_uri)
		free(event->common.edit_uri);
	event->common.edit_uri = updated.common.edit_uri;
	updated.common.edit_uri = NULL;

	if (event->common.etag)
		free(event->common.etag);
	event->common.etag = updated.common.etag;
	updated.common.etag = NULL;

	/* Cleanup updated event */
	gcal_destroy_entry(&updated);
exit:
	return result;

}

int gcal_erase_event(gcal_t gcal_obj, gcal_event_t event)
{
	int result = -1;
	if ((!gcal_obj) || (!event))
		goto exit;

	result = gcal_delete_event(gcal_obj, event);
exit:
	return result;
}

int gcal_get_updated_events(gcal_t gcal_obj, struct gcal_event_array *events,
			    char *timestamp)
{
	int result = -1;
	if (events)
		events->length = 0;

	if ((!gcal_obj) || (!events))
		return result;

	result = gcal_query_updated(gcal_obj, timestamp);
	if (result) {
		events->entries = NULL;
		events->length = 0;
		return result;
	}

	events->entries = gcal_get_entries(gcal_obj, &events->length);
	if (events->entries)
		result = 0;

	return result;
}

int gcal_get_events(gcal_t gcalobj, struct gcal_event_array *events_array)
{
	int result = -1;
	if (events_array)
		events_array->length = 0;

	if ((!gcalobj) || (!events_array))
		goto exit;

	result = gcal_dump(gcalobj);
	if (result == -1) {
		events_array->entries = NULL;
		events_array->length = 0;
		goto exit;
	}

	events_array->entries = gcal_get_entries(gcalobj, &events_array->length);
	if (!events_array->entries)
		result = -1;

exit:
	return result;
}

void gcal_cleanup_events(struct gcal_event_array *events)
{
	if (!events)
		return;

	gcal_destroy_entries(events->entries, events->length);
	events->length = 0;
	events->entries = NULL;
}

gcal_event_t gcal_event_element(struct gcal_event_array *events, size_t _index)
{
	struct gcal_event *event = NULL;
	if ((!events) || (_index > (events->length - 1)) ||
	    (events->length == 0))
		return event;

	event = &events->entries[_index];
	return event;
}

char *gcal_event_get_xml(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_xml(&(event->common));
}

char gcal_event_is_deleted(gcal_event_t event)
{
	if ((!event))
		return -1;
	return gcal_get_deleted(&(event->common));
}

char *gcal_event_get_id(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_id(&(event->common));
}

char *gcal_event_get_updated(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_updated(&(event->common));
}

char *gcal_event_get_title(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_title(&(event->common));
}

char *gcal_event_get_url(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_url(&(event->common));
}

char *gcal_event_get_etag(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return gcal_get_etag(&(event->common));
}

/* This are the fields unique to calendar events */
char *gcal_event_get_content(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->content;
}

char *gcal_event_get_recurrent(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->dt_recurrent;
}

char *gcal_event_get_start(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->dt_start;
}

char *gcal_event_get_end(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->dt_end;
}

char *gcal_event_get_where(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->where;
}

char *gcal_event_get_status(gcal_event_t event)
{
	if ((!event))
		return NULL;
	return event->status;
}


/* Here starts the setters */
int gcal_event_set_title(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->common.title)
		free(event->common.title);

	event->common.title = strdup(field);
	if (event->common.title)
		result = 0;

	return result;
}

int gcal_event_set_content(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->content)
		free(event->content);

	event->content = strdup(field);
	if (event->content)
		result = 0;

	return result;
}

int gcal_event_set_start(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->dt_start)
		free(event->dt_start);

	event->dt_start = strdup(field);
	if (event->dt_start)
		result = 0;

	return result;
}

int gcal_event_set_end(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->dt_end)
		free(event->dt_end);

	event->dt_end = strdup(field);
	if (event->dt_end)
		result = 0;

	return result;
}

int gcal_event_set_where(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->where)
		free(event->where);

	event->where = strdup(field);
	if (event->where)
		result = 0;

	return result;

}

int gcal_event_set_url(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->common.edit_uri)
		free(event->common.edit_uri);

	event->common.edit_uri = strdup(field);
	if (event->common.edit_uri)
		result = 0;

	return result;
}


int gcal_event_set_id(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->common.id)
		free(event->common.id);

	event->common.id = strdup(field);
	if (event->common.id)
		result = 0;

	return result;
}


int gcal_event_set_etag(gcal_event_t event, const char *field)
{
	int result = -1;

	if ((!event) || (!field))
		return result;

	if (event->common.etag)
		free(event->common.etag);

	event->common.etag = strdup(field);
	if (event->common.etag)
		result = 0;

	return result;
}
