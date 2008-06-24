/*
 * @file   utest_userapi.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on June 24 2008
 *
 * @brief  Header module for user api unit tests
 */

#include "utest_userapi.h"
#include "gcal.h"
#include "gcontact.h"
#include <stdio.h>

START_TEST (test_get_calendar)
{
	gcal gcal_obj;
	gcal_event event_array;
	size_t length;
	int result;

	gcal_obj = gcal_construct(GCALENDAR);
	fail_if(gcal_obj == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal_obj, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	event_array = gcal_get_events(gcal_obj, &length);
	fail_if(event_array == NULL, "Failed downloading events!");
	fail_if(length < 1, "gcal4tester must have at least 1 event!");

	/* Cleanup */
	gcal_destroy_entries(event_array, length);
	gcal_destroy(gcal_obj);

}
END_TEST


START_TEST (test_access_calendar)
{
	gcal gcal_obj;
	gcal_event event_array;
	size_t length, i;
	int result;
	char *ptr;

	gcal_obj = gcal_construct(GCALENDAR);
	result = gcal_get_authentication(gcal_obj, "gcal4tester", "66libgcal");
	event_array = gcal_get_events(gcal_obj, &length);

	/* Access events properties */
	for (i = 0; i < length; ++i) {
		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_get_calendar_id(event_array, i);
		ptr = gcal_get_calendar_updated(event_array, i);
		ptr = gcal_get_calendar_title(event_array, i);
		ptr = gcal_get_calendar_url(event_array, i);

		/* This are the fields unique to calendar events */
		ptr = gcal_get_calendar_content(event_array, i);
		ptr = gcal_get_calendar_recurrent(event_array, i);
		ptr = gcal_get_calendar_start(event_array, i);
		ptr = gcal_get_calendar_end(event_array, i);
		ptr = gcal_get_calendar_where(event_array, i);
		ptr = gcal_get_calendar_status(event_array, i);
		fail_if(ptr == NULL, "Can't get event field!");
	}

	ptr = gcal_get_calendar_title(event_array, length);
	fail_if(ptr != NULL, "Getting field must fail!");

	/* Cleanup */
	gcal_destroy_entries(event_array, length);
	gcal_destroy(gcal_obj);
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
	return tc;
}
