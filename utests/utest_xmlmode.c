/*
 * @file   utest_xmlmode.h
 *  @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Wed Jul 23 14:03:31 2008
 *
 * @brief  Test module for XML operating mode functions for libgcal.
 *
 *
 */

#include "utest_xmlmode.h"
#include "gcalendar.h"
#include "gcontact.h"
#include <stdio.h>
#include <string.h>


START_TEST (test_get_xmlentries)
{

	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event event;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	/* Set flag to save XML in internal field of each event */
	gcal_set_store_xml(gcal, 1);

	result = gcal_get_events(gcal, &event_array);
	fail_if(result == -1, "Failed downloading events!");

	event = gcal_event_element(&event_array, 0);
	xml_entry = gcal_event_get_xml(event);

	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "<gd:who");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST


TCase *xmlmode_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 60;
	tc = tcase_create("xmlmode");

	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_get_xmlentries);

	return tc;
}
