
#include "utest_query.h"
#include "gcal.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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
	int result, flag = 1;
	size_t length, i;
	struct gcal_event event, edit;
	struct gcal_event *entries = NULL;
	char *msg = NULL, current_timestamp[30];

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
	 * last updated right now.
	 */
	sleep(10);
	result = get_mili_timestamp(current_timestamp,
					       sizeof(current_timestamp),
					       NULL);
	if (result == -1) {
		msg = "Cannot create timestamp!";
		goto cleanup;
	}
	result = gcal_query_updated(ptr_gcal, current_timestamp);
	if (result == -1) {
		msg = "Failed querying!";
		goto cleanup;
	}

	entries = gcal_get_entries(ptr_gcal, &length);
	if (entries != NULL) {
		msg = "Query returned inconsistent results!";
		goto cleanup;
	}

	/* A query with NULL will use current day, starting by 06:00AM */
	result = gcal_query_updated(ptr_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying!";
		goto cleanup;
	}

	entries = gcal_get_entries(ptr_gcal, &length);
	if(entries == NULL) {
		msg = "Query returned inconsistent results!";
		goto cleanup;
	}

	for (i = 0; i < length; ++i)
		if (!(strcmp(entries[i].title, event.title))) {
			flag = 0;
			fprintf(stderr, "title is = %s\tlength = %d\n",
				entries[i].title, length);
			goto cleanup;
		}

	msg = "Cannot find newly added event!";


cleanup:
	result = gcal_delete_event(ptr_gcal, &edit);
	gcal_destroy_entries(entries, length);
	gcal_destroy_entry(&edit);
	fail_if(flag, msg);

}
END_TEST

TCase *gcal_query_tcase_create(void)
{

	TCase *tc = NULL;
	int timeout_seconds = 90;
	tc = tcase_create("gqueries");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_query_updated);

	return tc;
}
