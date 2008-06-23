
#include "utest_query.h"
#include "gcal.h"
#include "gcontact.h"
#include "gcal_status.h"
#include "internal_gcal.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_construct(GCALENDAR);
}

static void teardown(void)
{
	/* and here we clean up */
	gcal_destroy(ptr_gcal);
}

/* ATTENTION: this test will only succeed if *no* calendar operations
 * were done in this day.
 * This simulates the case where no new changes were done in the calendar
 * and we are asking for then (i.e. think in a sync operation).
 */
START_TEST (test_query_updated)
{
	int result, flag = 1;
	size_t length, i;
	struct gcal_event event, edit;
	struct gcal_event *entries = NULL;
	char *msg = NULL, current_timestamp[30];

	gcal_init_event(&event);
	gcal_init_event(&edit);

	event.common.title = "A test for updated query";
	event.content = "I will insert a new event and query just for it";
	event.dt_start = "2008-06-18T20:00:00-04:00";
	event.dt_end = "2008-06-18T21:00:00-04:00";
	event.where = "Place is -4GMT";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");
	result = gcal_create_event(ptr_gcal, &event, &edit);
	fail_if(result == -1, "Failed creating a new event!");

	/* This must return no results, since this user doesn't have
	 * calendar events last updated *right now*.
	 */
	result = get_mili_timestamp(current_timestamp,
				    sizeof(current_timestamp),
				    "-04:00");
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
	if (length != 0) {
		msg = "Query returned results!";
		goto cleanup;
	}

	/* A query with NULL will use current day, starting by 06:00AM plus
	 * the timezone.
	 */
	result = gcal_set_timezone(ptr_gcal, "-04:00");
	result = gcal_query_updated(ptr_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying!";
		goto cleanup;
	}

	entries = gcal_get_entries(ptr_gcal, &length);
	if((entries == NULL) || (length > 1)) {
		msg = "Query returned inconsistent results!";
		goto cleanup;
	}

	for (i = 0; i < length; ++i)
		if (!(strcmp(entries[i].common.title, event.common.title))) {
			flag = 0;
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

START_TEST (test_query_nulltz)
{
	int result, flag = 1;
	char *msg = NULL;
	size_t length;
	struct gcal_event *entries = NULL;

	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

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

	flag = 0;

cleanup:
	gcal_destroy_entries(entries, length);
	fail_if(flag, msg);
}
END_TEST

/* ATTENTION: this test will only succeed if *no* calendar operations
 * were done in this day.
 * This simulates the case where no new changes were done in the calendar
 * and we are asking for then (i.e. think in a sync operation).
 */
START_TEST (test_query_locationtz)
{
	int result, flag = 1;
	char *msg = NULL;
	size_t length;
	struct gcal_event *entries = NULL;

	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_set_timezone(ptr_gcal, "-04:00");
	result = gcal_set_location(ptr_gcal, "America/Manaus");

	result = gcal_query_updated(ptr_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying!";
		goto cleanup;
	}

	entries = gcal_get_entries(ptr_gcal, &length);
	if((entries == NULL) || (length > 1)) {
		msg = "Query returned inconsistent results!";
		goto cleanup;
	}

	flag = 0;

cleanup:
	gcal_destroy_entries(entries, length);
	/* Dirt trick to make sure that cleanup code for
	 * timezone+location is executed 8-)
	 */
	if (flag)
		gcal_destroy(ptr_gcal);
	fail_if(flag, msg);

}
END_TEST


START_TEST (test_query_contact)
{
	int result, flag = 0;
	struct gcal_contact contact, updated;
	struct gcal_resource *obj_gcal = NULL;
	size_t count;
	struct gcal_contact *contacts = NULL;
	char *msg = NULL;

	gcal_init_contact(&contact);
	gcal_init_contact(&updated);

	contact.title = "John Doe Query";
	contact.email = "john.doe.query@foo.bar.com";

	obj_gcal = gcal_construct(GCONTACT);
	fail_if(obj_gcal == NULL, "Failed to create gcal resource!");

	result = gcal_get_authentication(obj_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_contact(obj_gcal, &contact, &updated);
	fail_if(result == -1, "Failed creating a new contact!");

	result = gcal_query_updated(obj_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying for updated contacts!";
		flag = 1;
		goto cleanup;
	}

	contacts = gcal_get_contacts(obj_gcal, &count);
	if(contacts == NULL) {
		msg = "Failed extracting the contacts vector!";
		flag = 1;
		goto cleanup;
	}

	/* Google contacts *dont* display deleted contacts by default */
	if(count > 1) {
		msg = "Query returned inconsistent results!";
		flag = 1;
		goto cleanup;
	}

cleanup:

	gcal_delete_contact(obj_gcal, &updated);
	gcal_destroy_contact(&updated);
	gcal_destroy_contacts(contacts, count);
	gcal_destroy(obj_gcal);

	fail_if(flag, msg);
}
END_TEST


START_TEST (test_query_delcontact)
{
	int result, flag = 0;
	struct gcal_contact contact;
	struct gcal_contact *contacts = NULL;
	struct gcal_resource *obj_gcal = NULL;
	size_t count = 0;
	char *msg = NULL;

	gcal_init_contact(&contact);

	contact.title = "John Doe Query";
	contact.email = "john.doe.query@foo.bar.com";

	obj_gcal = gcal_construct(GCONTACT);
	fail_if(obj_gcal == NULL, "Failed to create gcal resource!");

	result = gcal_get_authentication(obj_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	/* Setting for deleted contacts should display at least
	 * one contact (the one added by 'test_query_contact'
	 * unit test).
	 */
	gcal_deleted(obj_gcal, SHOW);
	result = gcal_query_updated(obj_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying for updated contacts!";
		flag = 1;
		goto cleanup;
	}

	contacts = gcal_get_contacts(obj_gcal, &count);
	if((count < 1) || (contacts == NULL)) {
		msg = "Query didn't return deleted contacts!";
		flag = 1;
		goto cleanup;
	}

	/* Default will not show deleted contacts */
	gcal_destroy_contacts(contacts, count);
	gcal_deleted(obj_gcal, HIDE);
	result = gcal_query_updated(obj_gcal, NULL);
	if (result == -1) {
		msg = "Failed querying for updated contacts!";
		flag = 1;
		goto cleanup;
	}

	contacts = gcal_get_contacts(obj_gcal, &count);
	if(count > 1) {
		msg = "Query returned inconsistent results!";
		flag = 1;
		goto cleanup;
	}


cleanup:

	gcal_destroy_contacts(contacts, count);
	gcal_destroy(obj_gcal);

	fail_if(flag, msg);
}
END_TEST

START_TEST (test_query_generic)
{

	int result, flag = 0;
	struct gcal_contact *contacts = NULL;
	struct gcal_resource *obj_gcal = NULL;
	size_t count = 0;
	char *msg = NULL;
	char *query="updated-min=2008-06-20T06:00:00Z&"
	  "alt=atom&max-results=500&showdeleted=true";

	obj_gcal = gcal_construct(GCONTACT);
	fail_if(obj_gcal == NULL, "Failed to create gcal resource!");

	result = gcal_get_authentication(obj_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_query(obj_gcal, query);
	if ((result == -1) || (gcal_status_httpcode(obj_gcal) != 200)) {
		msg = "Failed using generic query!";
		flag = 1;
		goto cleanup;
	}

	contacts = gcal_get_contacts(obj_gcal, &count);
	if((count < 1) || (contacts == NULL)) {
		msg = "Query returned more contacts!";
		flag = 1;
		goto cleanup;
	}

cleanup:

	gcal_destroy_contacts(contacts, count);
	gcal_destroy(obj_gcal);

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
	tcase_add_test(tc, test_query_nulltz);
	tcase_add_test(tc, test_query_locationtz);
	tcase_add_test(tc, test_query_contact);
	tcase_add_test(tc, test_query_delcontact);
	tcase_add_test(tc, test_query_generic);
	return tc;
}
