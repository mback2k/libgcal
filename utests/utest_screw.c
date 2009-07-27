/*
 * @file   utest_screw.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Fri Aug  1 17:36:44 2008
 *
 * @brief  Utest trying to break the library.
 *
 * Tests with bad practices and forcing misuse of libgcal goes here.
 */

#include "utest_screw.h"
#include "gcalendar.h"
#include "gcontact.h"
/* XXX: API violation for forcing an error condition in 'mount_query_url' */
#include "internal_gcal.h"
#include "atom_parser.h"
#include <string.h>

START_TEST (test_normalize_url)
{
	char *copy;
	const char * const added = "http://www.google.com/calendar/feeds/"
		"default/private/full/ujq52gb0lggdjb0qqi10nt07m8";

	char retrieved[] = "http://www.google.com/calendar/feeds/"
		"gcalntester%40gmail.com/private/full/ujq52gb0lggdjb0qqi10nt07m8";
	workaround_edit_url(retrieved);
	fail_if(strcmp(added, retrieved) != 0, "String is not normalized!");

	copy = strdup(added);
	workaround_edit_url(copy);
	fail_if(strcmp(copy, added) != 0, "String should be equal!");
	free(copy);

}
END_TEST


START_TEST (test_usercalendarapi)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event_t event;
	char *ptr;
	int result;
	size_t i;

	/* Wrong object construction */
	gcal = gcal_new(1345);
	fail_if(gcal != NULL, "Should return NULL!");
	result = gcal_get_authentication(gcal, "nonexistant", "invalid");
	fail_if(result != -1, "Should fail authentication!");
	gcal_delete(gcal);

	/* Querying without authentication/username */
	gcal = gcal_new(GCALENDAR);
	result = gcal_get_updated_events(gcal, &event_array, NULL);
	fail_if(result != -1, "Should fail querying updated!");
	result = gcal_get_events(gcal, &event_array);
	fail_if(result != -1, "Should fail querying!");
	/* XXX: forcing test of internal 'mount_query_url' */
	gcal->auth = "foobie";
	result = gcal_get_updated_events(gcal, &event_array, NULL);
	fail_if(result != -1, "Should fail querying updated!");
	gcal->auth = NULL;
	gcal_delete(gcal);


	/* Failed authentication */
	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "nonexistant", "invalid");
	fail_if(result != -1, "Should fail authentication!");
	result = gcal_get_events(gcal, NULL);
	fail_if(result != -1, "Should event extraction!");
	result = gcal_get_events(gcal, &event_array);
	fail_if(result != -1, "Should event extraction!");

	for (i = 0; i < event_array.length; ++i) {
		event = gcal_event_element(&event_array, i);
		fail_if(event != NULL, "Should return NULL!");
		ptr = gcal_event_get_id(event);
		fail_if(ptr != NULL, "Should return NULL!");
	}

	/* Forces access to elements */
	event = gcal_event_element(&event_array, -1);
	fail_if(event != NULL, "Should return NULL!");
	event = gcal_event_element(&event_array, 0);
	fail_if(event != NULL, "Should return NULL!");
	ptr = gcal_event_get_id(event);
	fail_if(ptr != NULL, "Should return NULL!");


	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_usermismatch)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	struct gcal_contact_array contact_array;
	gcal_event_t event;
	gcal_contact_t contact;
	int result;

	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_events(gcal, &contact_array);
	fail_if(result != -1, "Should fail writing contact out of a calendar!");
	contact = gcal_contact_element(&contact_array, 0);
	fail_if(contact != NULL, "Contact should be NULL!");

	gcal_set_service(gcal, GCONTACT);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_contacts(gcal, &event_array);
	fail_if(result != -1, "Should fail writing events out of contacts!");
	event = gcal_event_element(&event_array, 0);
	fail_if(event != NULL, "Event should be NULL!");

}
END_TEST

TCase *gcal_screw(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcalscrew");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_usercalendarapi);
	tcase_add_test(tc, test_usermismatch);
	tcase_add_test(tc, test_normalize_url);
	return tc;
}
