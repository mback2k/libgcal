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

START_TEST (test_get_xmlcontacts)
{

	gcal_t gcal;
	struct gcal_contact_array contact_array;
	gcal_contact contact;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	/* Set flag to save XML in internal field of each event */
	gcal_set_store_xml(gcal, 1);

	result = gcal_get_contacts(gcal, &contact_array);
	fail_if(result == -1, "Failed downloading events!");

	contact = gcal_contact_element(&contact_array, 0);
	xml_entry = gcal_contact_get_xml(contact);

	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "<gd:email");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_oper_xmlevents)
{
	gcal_t gcal;
	gcal_event event;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	/* Set flag to save XML in internal field of each event */
	gcal_set_store_xml(gcal, 1);

	/* Create a new event object */
	event = gcal_event_new();
	fail_if (!event, "Cannot construct event object!");
	gcal_event_set_title(event, "A new event");
	gcal_event_set_content(event, "Here goes the description");
	gcal_event_set_start(event, "2008-06-24T16:00:00Z");
	gcal_event_set_end(event, "2008-06-24T18:00:00Z");
	gcal_event_set_where(event, "A nice place for a meeting");

	/* Add a new event */
	result = gcal_add_event(gcal, event);
	fail_if(result == -1, "Failed adding a new event!");
	xml_entry = gcal_event_get_xml(event);
	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "<gd:who");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");


	/* Edit this event */
	gcal_event_set_title(event, "Changing the title");
	result = gcal_update_event(gcal, event);
	fail_if(result == -1, "Failed editing event!");
	xml_entry = gcal_event_get_xml(event);
	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "Changing the title");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");


	/* Delete this event (note: google doesn't really deletes
	 * the event, but set its status to 'cancelled' and keeps
	 * then for nearly 4 weeks).
	 */
	result = gcal_erase_event(gcal, event);
	fail_if(result == -1, "Failed deleting event!");

	/* Cleanup */
	gcal_event_delete(event);
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
	tcase_add_test(tc, test_get_xmlcontacts);
	tcase_add_test(tc, test_oper_xmlevents);
	return tc;
}
