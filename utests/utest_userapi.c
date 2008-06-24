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


TCase *gcal_userapi(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcaluserapi");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_get_calendar);

	return tc;
}
