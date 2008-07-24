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


gcal_t gcal_new(gservice mode)
{
	return gcal_construct(mode);
}

void gcal_delete(gcal_t gcal_obj)
{
	gcal_destroy(gcal_obj);
}



gcal_event gcal_event_new(void)
{
	gcal_event result = NULL;
	result = malloc(sizeof(struct gcal_event));

	if (!result)
		return result;

	gcal_init_event(result);
	return result;
}

void gcal_event_delete(gcal_event event)
{

	if (!event)
		return;

	gcal_destroy_entry(event);
	free(event);
}

int gcal_add_xmlentry(gcal_t gcal_obj, char *xml_entry)
{
	(void)gcal_obj;
	(void)xml_entry;
	return -1;
}

int gcal_update_xmlentry(gcal_t gcal_obj, char *xml_entry)
{
	(void)gcal_obj;
	(void)xml_entry;
	return -1;
}

int gcal_erase_xmlentry(gcal_t gcal_obj, char *xml_entry)
{
	(void)gcal_obj;
	(void)xml_entry;
	return -1;
}


int gcal_add_event(gcal_t gcal_obj, gcal_event event)
{
	int result = -1;
	struct gcal_event updated;

	if ((!gcal_obj) || (!event))
		goto exit;

	result = gcal_create_event(gcal_obj, event, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: id, edit_uri and updated */
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

	/* Cleanup updated event */
	gcal_destroy_entry(&updated);
exit:
	return result;
}

int gcal_update_event(gcal_t gcal_obj, gcal_event event)
{
	int result = -1;
	struct gcal_event updated;

	if ((!gcal_obj) || (!event))
		goto exit;

	result = gcal_edit_event(gcal_obj, event, &updated);
	if (result)
		goto exit;

	/* Swap updated fields: edit_uri and updated */
	if (event->common.updated)
		free(event->common.updated);
	event->common.updated = updated.common.updated;
	updated.common.updated = NULL;

	if (event->common.edit_uri)
		free(event->common.edit_uri);
	event->common.edit_uri = updated.common.edit_uri;
	updated.common.edit_uri = NULL;

	/* Cleanup updated event */
	gcal_destroy_entry(&updated);
exit:
	return result;

}

int gcal_erase_event(gcal_t gcal_obj, gcal_event event)
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

gcal_event gcal_event_element(struct gcal_event_array *events, size_t _index)
{
	struct gcal_event *event = NULL;
	if ((!events) || (_index > (events->length - 1)) ||
	    (events->length == 0))
		return event;

	event = &events->entries[_index];
	return event;
}

char *gcal_event_get_xml(gcal_event event)
{
	if ((!event))
		return NULL;
	return gcal_get_xml(&(event->common));
}

char *gcal_event_get_id(gcal_event event)
{
	if ((!event))
		return NULL;
	return gcal_get_id(&(event->common));
}

char *gcal_event_get_updated(gcal_event event)
{
	if ((!event))
		return NULL;
	return gcal_get_updated(&(event->common));
}

char *gcal_event_get_title(gcal_event event)
{
	if ((!event))
		return NULL;
	return gcal_get_title(&(event->common));
}

char *gcal_event_get_url(gcal_event event)
{
	if ((!event))
		return NULL;
	return gcal_get_url(&(event->common));
}

/* This are the fields unique to calendar events */
char *gcal_event_get_content(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->content;
}

char *gcal_event_get_recurrent(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->dt_recurrent;
}

char *gcal_event_get_start(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->dt_start;
}

char *gcal_event_get_end(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->dt_end;
}

char *gcal_event_get_where(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->where;
}

char *gcal_event_get_status(gcal_event event)
{
	if ((!event))
		return NULL;
	return event->status;
}


/* Here starts the setters */
int gcal_event_set_title(gcal_event event, char *field)
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

int gcal_event_set_content(gcal_event event, char *field)
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

int gcal_event_set_start(gcal_event event, char *field)
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

int gcal_event_set_end(gcal_event event, char *field)
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

int gcal_event_set_where(gcal_event event, char *field)
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
