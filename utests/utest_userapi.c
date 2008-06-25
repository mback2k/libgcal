/*
 * @file   utest_userapi.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on June 24 2008
 *
 * @brief  Header module for user api unit tests
 */

#include "utest_userapi.h"
#include "gcalendar.h"
#include "gcontact.h"
#include <stdio.h>

START_TEST (test_get_calendar)
{
	gcal gcal_obj;
	struct gcal_entry_array event_array;
	int result;

	gcal_obj = gcal_construct(GCALENDAR);
	fail_if(gcal_obj == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal_obj, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	result = gcal_get_events(gcal_obj, &event_array);
	fail_if(result == -1, "Failed downloading events!");
	fail_if(event_array.length < 1, "gcal4tester must have at least"
		"1 event!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_destroy(gcal_obj);

}
END_TEST


START_TEST (test_access_calendar)
{
	gcal gcal_obj;
	struct gcal_entry_array event_array;
	gcal_event event;
	size_t i;
	int result;
	char *ptr;

	gcal_obj = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal_obj, "gcal4tester", "66libgcal");
	result = gcal_get_events(gcal_obj, &event_array);

	/* Access events properties */
	for (i = 0; i < event_array.length; ++i) {

		/* Access i-nth calendar event */
		event = gcal_event_element(&event_array, i);

		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_get_calendar_id(event);
		ptr = gcal_get_calendar_updated(event);
		ptr = gcal_get_calendar_title(event);
		ptr = gcal_get_calendar_url(event);

		fail_if(ptr == NULL, "Can't get edit url!");

		/* This are the fields unique to calendar events */
		ptr = gcal_get_calendar_content(event);
		ptr = gcal_get_calendar_recurrent(event);
		ptr = gcal_get_calendar_start(event);
		ptr = gcal_get_calendar_end(event);
		ptr = gcal_get_calendar_where(event);
		ptr = gcal_get_calendar_status(event);
	}

	/* This one is for testing purposes only! Please dont use
	 * gcal in this way.
	 */
	ptr = gcal_get_calendar_id(gcal_event_element(&event_array,
						      event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_updated(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_title(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_url(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_content(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_recurrent(gcal_event_element(&event_array,
							     event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_start(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_end(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_where(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_get_calendar_status(gcal_event_element(&event_array,
							  event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");


	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal_obj);
}
END_TEST


START_TEST (test_oper_calendar_event)
{
	gcal gcal_obj;
	gcal_event event;
	int result;

	/* Create a new event object */
	event = gcal_event_construct();
	fail_if (!event, "Cannot construct event object!");
	gcal_set_calendar_title(event, "A new event");
	gcal_set_calendar_content(event, "Here goes the description");
	gcal_set_calendar_start(event, "2008-06-24T16:00:00Z");
	gcal_set_calendar_end(event, "2008-06-24T18:00:00Z");
	gcal_set_calendar_where(event, "A nice place for a meeting");

	/* Create a gcal object and authenticate */
	gcal_obj = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal_obj, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Add a new event */
	result = gcal_add_event(gcal_obj, event);
	fail_if(result == -1, "Failed adding a new event!");


	/* Edit this event */
	gcal_set_calendar_title(event, "Changing the title");
	result = gcal_update_event(gcal_obj, event);
	fail_if(result == -1, "Failed editing event!");

	/* Delete this event (note: google doesn't really deletes
	 * the event, but set its status to 'cancelled' and keeps
	 * then for nearly 4 weeks).
	 */
	result = gcal_erase_event(gcal_obj, event);
	fail_if(result == -1, "Failed deleting event!");

	/* Cleanup */
	gcal_event_destroy(event);
	gcal_delete(gcal_obj);
}
END_TEST


TCase *gcal_userapi(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcaluserapi");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_get_calendar);
	tcase_add_test(tc, test_access_calendar);
	tcase_add_test(tc, test_oper_calendar_event);
	return tc;
}
