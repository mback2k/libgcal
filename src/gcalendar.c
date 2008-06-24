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


gcal_event gcal_event_construct(void)
{
	gcal_event result = NULL;
	result = malloc(sizeof(struct gcal_event));

	if (!result)
		return result;

	gcal_init_event(result);
	return result;
}

void gcal_event_destroy(gcal_event event)
{

	if (!event)
		return;

	gcal_destroy_entry(event);
	free(event);
}

int gcal_add_event(gcal gcal_obj, gcal_event event)
{
	int result = -1;
	if ((!gcal_obj) || (!event))
		goto exit;

exit:
	return result;
}

int gcal_update_event(gcal gcal_obj, gcal_event event)
{
	int result = -1;
	if ((!gcal_obj) || (!event))
		goto exit;

exit:
	return result;

}

int gcal_erase_event(gcal gcal_obj, gcal_event event)
{
	int result = -1;
	if ((!gcal_obj) || (!event))
		goto exit;

exit:
	return result;
}

int gcal_get_events(gcal gcalobj, struct gcal_entry_array *events_array)
{
	int result = -1;
	if ((!gcalobj) || (!events_array))
		goto exit;

	result = gcal_dump(gcalobj);
	if (result == -1)
		goto exit;

	events_array->entries = gcal_get_entries(gcalobj, &events_array->length);

exit:
	return result;
}

void gcal_cleanup_events(struct gcal_entry_array *events)
{
	if (!events)
		return;

	gcal_destroy_entries(events->entries, events->length);
	events->length = 0;
	events->entries = NULL;
}

char *gcal_get_calendar_id(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return gcal_get_id(&(event[_index]).common);
}

char *gcal_get_calendar_updated(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return gcal_get_updated(&(event[_index]).common);
}

char *gcal_get_calendar_title(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return gcal_get_title(&(event[_index]).common);
}

char *gcal_get_calendar_url(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return gcal_get_url(&(event[_index]).common);
}

/* This are the fields unique to calendar events */
char *gcal_get_calendar_content(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].content;
}

char *gcal_get_calendar_recurrent(struct gcal_entry_array *events,
				  size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].dt_recurrent;
}

char *gcal_get_calendar_start(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].dt_start;
}

char *gcal_get_calendar_end(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].dt_end;
}

char *gcal_get_calendar_where(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].where;
}

char *gcal_get_calendar_status(struct gcal_entry_array *events, size_t _index)
{
	struct gcal_event *event;
	if ((!events) || (_index > (events->length - 1)))
		return NULL;

	event = events->entries;
	return event[_index].status;
}


/* Here starts the setters */
int gcal_set_calendar_title(gcal_event event, char *field)
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

int gcal_set_calendar_content(gcal_event event, char *field)
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

int gcal_set_calendar_start(gcal_event event, char *field)
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

int gcal_set_calendar_end(gcal_event event, char *field)
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

int gcal_set_calendar_where(gcal_event event, char *field)
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
