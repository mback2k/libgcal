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
#include "utils.h"
#include "gcal_status.h"
#include "gcontact.h"

START_TEST (test_get_xmlentries)
{

	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event_t event;
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
	gcal_contact_t contact;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
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
	gcal_event_t event;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	/* Set flag to save XML in internal field of each event */
	gcal_set_store_xml(gcal, 1);

	/* Create a new event object */
	event = gcal_event_new(NULL);
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

START_TEST (test_oper_xmlcontact)
{
	gcal_t gcal;
	gcal_contact_t contact;
	int result;
	char *xml_entry, *tmp;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	/* Set flag to save XML in internal field of each contact */
	gcal_set_store_xml(gcal, 1);

	/* Create a new contact object */
	contact = gcal_contact_new(NULL);
	fail_if(!contact, "Cannot construct contact object!");
	gcal_contact_set_title(contact, "Jonhy Generic Guy");
	gcal_contact_delete_email_addresses(contact);
	gcal_contact_add_email_address(contact, "johnthedoe@nobody.com", E_OTHER, 1);


	/* Add a new contact */
	result = gcal_add_contact(gcal, contact);
	fail_if(result == -1, "Failed adding a new contact!");
	xml_entry = gcal_contact_get_xml(contact);
	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "<gd:email");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");


	/* Edit this contact */
	gcal_contact_set_title(contact, "Jonhy Super Generic Guy");
	result = gcal_update_contact(gcal, contact);
	fail_if(result == -1, "Failed editing contact!");
	xml_entry = gcal_contact_get_xml(contact);
	fail_if(xml_entry == NULL, "Cannot access raw XML!");
	tmp = strstr(xml_entry, "Super");
	fail_if(xml_entry == NULL, "Raw XML lacks field!");


	/* Delete this contact (the contact can still be retrieved
	 * using query parameter 'showdeleted=true' for
	 * for nearly 4 weeks).
	 */
	result = gcal_erase_contact(gcal, contact);
	fail_if(result == -1, "Failed deleting contact!");

	/* Cleanup */
	gcal_contact_delete(contact);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_oper_purexml)
{
	char *super_contact = NULL, *edit_url = NULL, *etag = NULL;
	char *updated1 = NULL, *updated2 = NULL, *updated3 = NULL;
	gcal_t gcal;
	gcal_contact_t contact;
	int result;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	if (find_load_file("/utests/supercontact.xml", &super_contact))
		fail_if(1, "Cannot load contact XML file!");

	/* Add and update */
	result = gcal_add_xmlentry(gcal, super_contact, &updated1);
	fail_if(result == -1, "Failed adding a new contact! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	result = gcal_update_xmlentry(gcal, updated1, &updated2, NULL, NULL);
	fail_if(result == -1, "Failed editing a new contact! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* Create a contact object out of raw XML: useful to get the
	 * updated edit_url, id, etc.
	 */
	contact = gcal_contact_new(updated2);
	fail_if(!contact, "Cannot create contact object!\n");
	fail_if(strcmp("John 'Super' Doe", gcal_contact_get_title(contact)),
		"Failure parsing contact XML: title!\n");

	/* update corner case where the new XML doesn't have the edit URL */
	free(super_contact);
	if (find_load_file("/utests/contact_documentation.xml", &super_contact))
		fail_if(1, "Cannot load contact XML file!");

	result = gcal_get_edit_url(updated2, &edit_url);
	fail_if(result == -1, "Cannot extract edit URL!");
	result = gcal_get_extract_etag(updated2, &etag);
	fail_if(result == -1, "Cannot extract etag!");

	result = gcal_update_xmlentry(gcal, super_contact, &updated3,
				      edit_url, etag);
	fail_if(result == -1, "Failed editing a new contact! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* delete */
	result = gcal_erase_xmlentry(gcal, updated3);
	fail_if(result == -1, "Failed deleting a new contact! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* Cleanup */
	free(super_contact);
	free(updated1);
	free(updated2);
	free(updated3);
	free(edit_url);
	gcal_delete(gcal);
	gcal_contact_delete(contact);

}
END_TEST

START_TEST (test_oper_purexmlcal)
{
	char *super_calendar = NULL, *edit_url = NULL, *etag = NULL;
	char *updated1 = NULL, *updated2 = NULL, *updated3 = NULL;
	gcal_t gcal;
	gcal_event_t event;
	int result;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	if (find_load_file("/utests/fullcalendar.xml", &super_calendar))
		fail_if(1, "Cannot load calendar XML file!");

	/* Add and update */
	result = gcal_add_xmlentry(gcal, super_calendar, &updated1);
	fail_if(result == -1, "Failed adding a new calendar! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	result = gcal_update_xmlentry(gcal, updated1, &updated2, NULL, NULL);
	fail_if(result == -1, "Failed editing a new calendar! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* Create an event object out of raw XML: useful to get the
	 * updated edit_url, id, etc.
	 */
	event = gcal_event_new(updated2);
	fail_if(!event, "Cannot create event object!\n");
	fail_if(strcmp("Hockey with Beth", gcal_event_get_title(event)),
		"Failure parsing event XML: title!\n");


	/* update corner case: where the new XML doesn't have the edit URL */
	free(super_calendar);
	if (find_load_file("/utests/calendar_documentation.xml", &super_calendar))
		fail_if(1, "Cannot load calendar XML file!");

	result = gcal_get_edit_url(updated2, &edit_url);
	fail_if(result == -1, "Cannot extract edit URL!");
	result = gcal_get_extract_etag(updated2, &etag);
	fail_if(result == -1, "Cannot extract etag!");

	result = gcal_update_xmlentry(gcal, super_calendar, &updated3,
				      edit_url, etag);
	fail_if(result == -1, "Failed editing a new event! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* delete */
	result = gcal_erase_xmlentry(gcal, updated3);
	fail_if(result == -1, "Failed deleting a new calendar! HTTP code: %d"
		"\nmsg: %s\n", gcal_status_httpcode(gcal),
		gcal_status_msg(gcal));

	/* Cleanup */
	free(super_calendar);
	free(updated1);
	free(updated2);
	free(updated3);
	free(edit_url);
	gcal_delete(gcal);
	gcal_event_delete(event);

}
END_TEST

TCase *xmlmode_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 60;
	tc = tcase_create("xmlmode");

 	tcase_set_timeout(tc, timeout_seconds);
	tcase_add_test(tc, test_get_xmlentries);
	tcase_add_test(tc, test_get_xmlcontacts);
	tcase_add_test(tc, test_oper_xmlevents);
	tcase_add_test(tc, test_oper_xmlcontact);
	tcase_add_test(tc, test_oper_purexml);
	tcase_add_test(tc, test_oper_purexmlcal);
	return tc;
}
