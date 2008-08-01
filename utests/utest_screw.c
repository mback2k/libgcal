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


START_TEST (test_usercalendarapi)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event event;
	char *ptr;
	int result;
	size_t i;

	/* Wrong object construction */
	gcal = gcal_new(1345);
	fail_if(gcal != NULL, "Should return NULL!");

	/* Failed authentication */
	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "nonexistant", "invalid");
	fail_if(result != -1, "Should fail authentication!");
	result = gcal_get_events(gcal, &event_array);
	fail_if(result != -1, "Should event extraction!");

	for (i = 0; i < event_array.length; ++i) {
		event = gcal_event_element(&event_array, i);
		fail_if(event != NULL, "Should return NULL!");
		ptr = gcal_event_get_id(event);
		fail_if(ptr != NULL, "Should return NULL!");
	}


	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST

TCase *gcal_screw(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcalscrew");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_usercalendarapi);
	return tc;
}
