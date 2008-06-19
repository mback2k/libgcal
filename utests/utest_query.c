
#include "utest_query.h"
#include "gcal.h"
#include <string.h>

static struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_initialize(GCALENDAR);
}

static void teardown(void)
{
	/* and here we clean up */
	gcal_destroy(ptr_gcal);
}

START_TEST (test_query_updated)
{
	int result;
	size_t length, i;
	struct gcal_event event, edit;
	struct gcal_event *entries;

	gcal_init_event(&event);
	gcal_init_event(&edit);

	event.title = "A test for updated query";
	event.content = "I will insert a new event and query just for it";
	event.dt_start = "2008-06-18T20:00:00-04:00";
	event.dt_end = "2008-06-18T21:00:00-04:00";
	event.where = "Place is -4GMT";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Authentication should work.");
	result = gcal_create_event(ptr_gcal, &event, &edit);
	fail_if(result == -1, "Failed creating a new event!");

	/* This must fail, since this user doesn't have calendar events
	 * for 2020!
	 */
	result = gcal_query_updated(ptr_gcal, "2020-06-18T00:12:00");
	fail_if(result == -1, "Failed querying!");
	entries = gcal_get_entries(ptr_gcal, &length);
	fail_if(entries != NULL, "Query returned inconsistent results!");

	/* A query with NULL will use current day, starting by 06:00AM */
	result = gcal_query_updated(ptr_gcal, NULL);
	fail_if(result == -1, "Failed querying!");
	entries = gcal_get_entries(ptr_gcal, &length);
	fail_if(entries == NULL, "Query returned inconsistent results!");

	for (i = 0; i < length; ++i)
		if (!(strcmp(entries[i].title, event.title)))
			goto cleanup;
	fail_if(1, "Cannot find newly added event!");


cleanup:
	result = gcal_delete_event(ptr_gcal, &edit);
	gcal_destroy_entries(entries, length);
	gcal_destroy_entry(&edit);

}
END_TEST

TCase *gcal_query_tcase_create(void)
{

	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gqueries");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_query_updated);

	return tc;
}
