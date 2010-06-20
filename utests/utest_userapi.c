/*
 * @file   utest_userapi.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on June 24 2008
 *
 * @brief  Implementation module for user api unit tests.
 *
 * This is a good place to look when learning to use libgcal. The following
 * operations are covered:
 * - authentication
 * - getting all calendar events
 * - accessing them
 * - adding a new calendar event
 * - editing and deleting an event
 * - querying for updated calendar events
 *
 */

#include "utest_userapi.h"
#include "gcalendar.h"
#include "gcontact.h"
#include "gcal_parser.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/* I use this variable to exchange data between the contacts tests */
char *deleted_contact_id = NULL;

START_TEST (test_get_calendar)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	int result;

	gcal = gcal_new(GCALENDAR);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	result = gcal_get_events(gcal, &event_array);
	fail_if(result == -1, "Failed downloading events!");
	fail_if(event_array.length < 1, "gcal4tester must have at least"
		"1 event!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST


START_TEST (test_access_calendar)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event_t event;
	size_t i;
	int result;
	char *ptr;

	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_events(gcal, &event_array);

	/* Access events properties */
	for (i = 0; i < event_array.length; ++i) {

		/* Access i-nth calendar event */
		event = gcal_event_element(&event_array, i);

		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_event_get_id(event);
		ptr = gcal_event_get_updated(event);
		ptr = gcal_event_get_title(event);
		ptr = gcal_event_get_url(event);

		fail_if(ptr == NULL, "Can't get edit url!");

		/* This are the fields unique to calendar events */
		ptr = gcal_event_get_content(event);
		ptr = gcal_event_get_recurrent(event);
		ptr = gcal_event_get_start(event);
		ptr = gcal_event_get_end(event);
		ptr = gcal_event_get_where(event);
		ptr = gcal_event_get_status(event);
	}

	/* This code block is for testing overflow only! Please dont use
	 * gcal in this way.
	 */
	ptr = gcal_event_get_id(gcal_event_element(&event_array,
						      event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_updated(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_title(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_url(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_content(gcal_event_element(&event_array,
							   event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_recurrent(gcal_event_element(&event_array,
							     event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_start(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_end(gcal_event_element(&event_array,
						       event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_where(gcal_event_element(&event_array,
							 event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_event_get_status(gcal_event_element(&event_array,
							  event_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");


	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);
}
END_TEST


START_TEST (test_oper_event_event)
{
	gcal_t gcal;
	gcal_event_t event;
	int result;

	/* Create a new event object */
	event = gcal_event_new(NULL);
	fail_if (!event, "Cannot construct event object!");
	gcal_event_set_title(event, "A new event");
	gcal_event_set_content(event, "Here goes the description");
	gcal_event_set_start(event, "2008-06-24T16:00:00Z");
	gcal_event_set_end(event, "2008-06-24T18:00:00Z");
	gcal_event_set_where(event, "A nice place for a meeting");

	/* Create a gcal object and authenticate */
	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Add a new event */
	result = gcal_add_event(gcal, event);
	fail_if(result == -1, "Failed adding a new event!");


	/* Edit this event */
	gcal_event_set_title(event, "Changing the title");
	result = gcal_update_event(gcal, event);
	fail_if(result == -1, "Failed editing event!");

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

START_TEST (test_query_event_updated)
{
	gcal_t gcal;
	struct gcal_event_array event_array;
	gcal_event_t event;
	int result;
	/* Previous test added/edited/deleted an event with this title */
	char *title = "Changing the title";

	gcal = gcal_new(GCALENDAR);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");

	/* This will query for all updated events (fall in this category
	 * added/deleted/updated events) starting for 06:00Z UTC of today).
	 */
	result = gcal_get_updated_events(gcal, &event_array, NULL);
	fail_if(result == -1, "Failed downloading updated events!");
	fail_if(event_array.length < 1, "If previous test was ok, it must"
		" return at least one updated event!");

	/* Google returns the last updated event first */
	event = gcal_event_element(&event_array, 0);
	if (gcal_event_is_deleted(event)) {
		if (gcal_event_get_title(event))
			result = strcmp(gcal_event_get_title(event), title);
	} else
		result = -1;
	fail_if(result != 0, "Cannot locate event!");

	/* Cleanup */
	gcal_cleanup_events(&event_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_get_contacts)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	int result;

	gcal = gcal_new(GCONTACT);
	fail_if(gcal == NULL, "Failed constructing gcal object!");

	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	result = gcal_get_contacts(gcal, &contact_array);
	fail_if(result == -1, "Failed downloading contacts!");
	fail_if(contact_array.length != 3, "gcal4tester must have only"
		"3 contacts!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST


START_TEST (test_access_contacts)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	gcal_contact_t contact;
	size_t i;
	int result;
	char *ptr;
	int j;
	gcal_email_type get;
	gcal_phone_type gpt;
	gcal_im_type git;

	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcal4tester", "66libgcal");
	result = gcal_get_contacts(gcal, &contact_array);

	/* Access events properties */
	for (i = 0; i < contact_array.length; ++i) {

		/* Access i-nth calendar event */
		contact = gcal_contact_element(&contact_array, i);

		/* Common fields between calendar and contacts are
		 * of type 'gcal_entry'
		 */
		ptr = gcal_contact_get_id(contact);
		ptr = gcal_contact_get_updated(contact);
		/* Tip: it *is* valid a contact have no name. */
		ptr = gcal_contact_get_title(contact);
		ptr = gcal_contact_get_url(contact);

		fail_if(ptr == NULL, "Can't get edit url!");

		/* This are the fields unique to calendar events */
		j = gcal_contact_get_emails_count(contact);
		j = gcal_contact_get_pref_email(contact);
		ptr = gcal_contact_get_email_address(contact, 0);
		get = gcal_contact_get_email_address_type(contact, 0);
		ptr = gcal_contact_get_content(contact);
		ptr = gcal_contact_get_organization(contact);
		ptr = gcal_contact_get_profission(contact);
		j = gcal_contact_get_im_count(contact);
		ptr = gcal_contact_get_im_address(contact, 0);
		ptr = gcal_contact_get_im_protocol(contact, 0);
		git = gcal_contact_get_im_type(contact, 0);
		j = gcal_contact_get_phone_numbers_count(contact);
		ptr = gcal_contact_get_phone_number(contact, 0);
		gpt = gcal_contact_get_phone_number_type(contact, 0);
		ptr = gcal_contact_get_address(contact);

	}

	/* This code block is for testing overflow only! Please dont use
	 * gcal in this way.
	 */
	ptr = gcal_contact_get_id(gcal_contact_element(&contact_array,
						   contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_updated(gcal_contact_element(&contact_array,
							contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_title(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_url(gcal_contact_element(&contact_array,
						    contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_email_address(gcal_contact_element(&contact_array,
							contact_array.length), 0);
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_content(gcal_contact_element(&contact_array,
							contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_organization(gcal_contact_element(&contact_array,
							    contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_profission(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_im_address(gcal_contact_element(&contact_array,
						      contact_array.length), 0);
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_phone_number(gcal_contact_element(&contact_array,
							  contact_array.length), 0);
	fail_if(ptr != NULL, "Getting field must fail!");
	ptr = gcal_contact_get_address(gcal_contact_element(&contact_array,
						      contact_array.length));
	fail_if(ptr != NULL, "Getting field must fail!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_oper_contact)
{
	gcal_t gcal;
	gcal_contact_t contact;
	int result;

	/* Create a new contact object */
	contact = gcal_contact_new(NULL);
	fail_if (!contact, "Cannot construct contact object!");

	gcal_contact_set_title(contact, "John Doe");
	gcal_contact_set_email(contact, "john.doe@foo.bar.com");
	gcal_contact_add_email_address(contact, "jonny@theman.com", E_OTHER, 0);
	gcal_contact_set_phone(contact, "111-2222-3333-888");

	contact->structured_name_nr = 1;
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"givenName","John");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"familyName","Doe");
	
	gcal_contact_delete_email_addresses(contact);
	gcal_contact_add_email_address(contact, "john.doe@foo.bar.com", E_OTHER, 1);

	/* Create a gcal object and authenticate */
	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Add a new contact */
	result = gcal_add_contact(gcal, contact);
	fail_if(result == -1, "Failed adding a new contact!");


	/* Tests integraty */
	result = strcmp(gcal_contact_get_phone(contact), "111-2222-3333-888");
	fail_if(result != 0, "Failed to extract phone from gcal_contact_t!");


	/* Edit this contact */
// 	gcal_contact_set_title(contact, "John 'The Generic' Doe");
	
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"givenName","John");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"additionalName","'The Generic'");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"familyName","Doe");
	
	fail_if(result == -1, "Failed editing contact!");
	gcal_contact_delete_email_addresses(contact);
	gcal_contact_set_email(contact, "john.super.doe@foo.bar.com");
	fail_if(result == -1, "Failed editing contact!");
	result = gcal_update_contact(gcal, contact);
	fail_if(result == -1, "Failed uploading edited contact!");

	/* Save this contact's ID to use it in the next test, where we
	 * search for updated contacts.
	 */
	deleted_contact_id = strdup(gcal_contact_get_id(contact));

	/* Delete this contact (note: google still keeps a deleted contact
	 * for nearly 4 weeks. Its possible to retrieve it using
	 * 'gcal_deleted(gcal, SHOW)' before downloading contacts)
	 */
	result = gcal_erase_contact(gcal, contact);
	fail_if(result == -1, "Failed deleting contact!");

	/* Cleanup */
	gcal_contact_delete(contact);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_query_contact_updated)
{
	gcal_t gcal;
	struct gcal_contact_array contact_array;
	gcal_contact_t contact;
	int result;
	size_t tmp;

	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");

	/* This will query for all updated contacts (fall in this category
	 * added/updated contacts) starting for 06:00Z UTC of today).
	 */
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");
	fail_if(contact_array.length > 3, "This user should not have more"
		" than 3 updated contacts!");

	/* Now we query for deleted contacts (previous test
	 * added/updated/deleted one contact, remember?)
	 */
	tmp = contact_array.length;
	gcal_deleted(gcal, SHOW);
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");
	fail_if(contact_array.length <= tmp , "If previous test was ok, it must"
		" return one more contact!");

	/* FIXME: Contacts doesn't return the last updated contact
	 * first when running with 'showdeleted'.
	 */
	result = -1;
 	for (tmp = 0; tmp < contact_array.length; ++tmp) {
		contact = gcal_contact_element(&contact_array, tmp);
		/* only compare deleted contacts */
		if (gcal_contact_is_deleted(contact))
			result = strcmp(gcal_contact_get_id(contact),
					deleted_contact_id);
		if (!result)
			break;
	}

	fail_if(result != 0, "Cannot locate contact!");

	/* Cleanup */
	gcal_cleanup_contacts(&contact_array);
	gcal_delete(gcal);

}
END_TEST


START_TEST (test_contact_photo)
{
	gcal_t gcal;
	gcal_contact_t contact, tmp;
	char *photo_data;
	struct gcal_contact_array contact_array;
	int result;

	if (find_load_photo("/utests/images/gromit.jpg",  &photo_data, &result))
		fail_if(1, "Cannot load photo!");

	/* Create a new contact object */
	contact = gcal_contact_new(NULL);
	fail_if (!contact, "Cannot construct contact object!");
	gcal_contact_set_title(contact, "Gromit");
	gcal_contact_add_email_address(contact, "gromit@wallace.com", E_OTHER, 1);
	fail_if(gcal_contact_set_photo(contact, photo_data, result),
		"Failed copying photo data");

	/* Create a gcal object and authenticate */
	gcal = gcal_new(GCONTACT);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Failed getting authentication");

	/* Create a new contact with photo */
	result = gcal_add_contact(gcal, contact);
	fail_if(result == -1, "Failed adding a new contact!");

	/* Update the contact: new title, photo, name, etc */
	free(photo_data);
	photo_data = NULL;
	if (find_load_photo("/utests/images/hutch.png",  &photo_data, &result))
		fail_if(1, "Cannot load photo!");
	gcal_contact_set_title(contact, "hutch");
	gcal_contact_delete_email_addresses(contact);
	gcal_contact_add_email_address(contact, "hutch@wallace.com", E_OTHER, 1);
	fail_if(gcal_contact_set_photo(contact, photo_data, result),
		"Failed copying photo data");
	result = gcal_update_contact(gcal, contact);
	fail_if(result == -1, "Failed updating a contact!");


	/* Retrieve updated contacts and test for contact photo */
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");
/* 	fail_if(contact_array.length > 3, "This user should not have more" */
/* 		" than 3 updated contacts!"); */

	/* Last updated contact (i.e. last) should have photo */
	tmp = gcal_contact_element(&contact_array, (contact_array.length - 1));
	fail_if(tmp == NULL, "Last contact must not be NULL!");
	fail_if(gcal_contact_get_photo(tmp) == NULL,
		"Last updated contact must have photo: %s",
		gcal_contact_get_title(tmp));
	fail_if(gcal_contact_get_photolength(tmp) < 2,
		"Last updated contact photo length must be bigger");

	/* Delete */
	result = gcal_erase_contact(gcal, contact);
	fail_if(result == -1, "Failed deleting contact!");

	/* Cleanup */
	gcal_contact_delete(contact);
	gcal_delete(gcal);

	gcal_cleanup_contacts(&contact_array);
	free(photo_data);
}
END_TEST

START_TEST (test_url_sanity_calendar)
{
	gcal_t gcal;
	gcal_event_t event;
	struct gcal_event_array all_events;
	int result;
	event = gcal_event_new(NULL);

	gcal = gcal_new(GCALENDAR);
	gcal_set_store_xml(gcal, 1);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Cannot authenticate!");

	char start[] = "2009-03-26T11:00:00.000Z";
	char end[] = "2009-03-26T12:00:00.000Z";
  	gcal_event_set_title(event, "Insanity in edit URL");
	gcal_event_set_content(event, "I'm bored of gcalendar bugs");
	gcal_event_set_where(event, "someplace");
	gcal_event_set_start(event, start);
	gcal_event_set_end(event, end);

	fail_if((result = gcal_add_event(gcal, event)) != 0,
		"Failed adding new event!");
	fail_if((result = gcal_get_events(gcal, &all_events)) != 0,
		 "Failed retrieving all events!");
	fail_if((strcmp(gcal_event_get_url(event),
			gcal_event_get_url(gcal_event_element(&all_events, 0)))
		 != 0), "Edit url is different!");

/* 	fprintf(stderr, "add: %s\nretrieve: %s\n", gcal_event_get_url(event), */
/* 		gcal_event_get_url(gcal_event_element(&all_events, 0))); */

	fail_if((result = gcal_erase_event(gcal, event)) != 0,
		"Failed deleting test event!");

	gcal_event_delete(event);
	gcal_cleanup_events(&all_events);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_url_sanity_contact)
{
	gcal_t gcal;
	gcal_contact_t contact;
	struct gcal_contact_array all_contacts;
	int result;
	contact = gcal_contact_new(NULL);

	gcal = gcal_new(GCONTACT);
	gcal_set_store_xml(gcal, 1);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Cannot authenticate!");

  	gcal_contact_set_title(contact, "Insanity in edit URL");
	gcal_contact_add_email_address(contact, "prooftest@add.get.com", E_OTHER, 1);

	fail_if((result = gcal_add_contact(gcal, contact)) != 0,
		"Failed adding new contact!");
	fail_if((result = gcal_get_contacts(gcal, &all_contacts)) != 0,
		 "Failed retrieving all contacts!");
	fail_if((strcmp(gcal_contact_get_url(contact),
			gcal_contact_get_url(gcal_contact_element(&all_contacts,  all_contacts.length - 1)))
		 != 0), "Edit url is different!");

/* 	fprintf(stderr, "add: %s\nretrieve: %s\n", */
/* 		gcal_contact_get_url(contact), */
/* 		gcal_contact_get_url(gcal_contact_element(&all_contacts,  all_contacts.length - 1))); */

	fail_if((result = gcal_erase_contact(gcal, contact)) != 0,
		"Failed deleting test contact!");

	gcal_contact_delete(contact);
	gcal_cleanup_contacts(&all_contacts);
	gcal_delete(gcal);

}
END_TEST

START_TEST (test_contact_new_fields)
{

	gcal_t gcal;
	int result, address_nr, address_count;
	struct gcal_contact_array contact_array;
	gcal_contact_t contact, contact_temp;
	gcal_structured_subvalues_t structured_entry;
	char *temp;
	size_t i;

	/* Create a new contact object */
	contact = gcal_contact_new(NULL);
	fail_if (!contact, "Cannot construct contact object!");

	contact->structured_name=(struct gcal_structured_subvalues *)malloc(sizeof(struct gcal_structured_subvalues));
	contact->structured_name->field_typenr = 0;
	contact->structured_name->field_key = malloc(sizeof(char*));
	contact->structured_name->field_key = NULL;
	contact->structured_name->field_value = malloc(sizeof(char*));
	contact->structured_name->field_value = NULL;
	contact->structured_name_nr = 1;
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"givenName","Johnny");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"additionalName","W.");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"familyName","Doe");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"namePrefix","Dr.");

	/* extra fields */
	gcal_contact_set_nickname(contact,"The Fox");
	gcal_contact_set_occupation(contact,"Programmer");
	gcal_contact_set_birthday(contact,"1963-11-11");
	gcal_contact_add_im(contact,"SKYPE","johnny_skype",I_HOME,1);
	gcal_contact_add_im(contact,"AIM","johnny_aim",I_HOME,0);
	gcal_contact_set_homepage(contact,"www.homegage.com");
	gcal_contact_set_blog(contact,"myblog.homegage.com");

	contact->structured_address=(struct gcal_structured_subvalues *)malloc(sizeof(struct gcal_structured_subvalues));
	contact->structured_address->field_typenr = 0;
	contact->structured_address->field_key = malloc(sizeof(char*));
	contact->structured_address->field_key = NULL;
	contact->structured_address->field_value = malloc(sizeof(char*));
	contact->structured_address->field_value = NULL;
	contact->structured_address_nr = 0;
	contact->structured_address_type = (char**)malloc(sizeof(char*));
	address_nr = gcal_contact_set_structured_address_nr(contact,A_HOME);
	address_count = contact->structured_address_nr;
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"street","Unknown Av St, n 69");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"pobox","PO BOX 123 456");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"city","Dirty Old Town");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"region","Somewhere");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"postcode","ABC 12345-D");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"country","Madagascar");
	address_nr = gcal_contact_set_structured_address_nr(contact,A_WORK);
	address_count = contact->structured_address_nr;
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"street","This One St, n 23");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"pobox","PO BOX 333 444 5");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"city","My Hometown");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"region","Hereorthere");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"postcode","XYZ 98765-C");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"country","Island");
	gcal_contact_set_pref_structured_address(contact,1);

	gcal = gcal_new(GCONTACT);
        gcal_set_store_xml(gcal, 1);
	result = gcal_get_authentication(gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_add_contact(gcal, contact);
	fail_if(result == -1, "Failed adding a new contact!");

	/* Edit the new contact */
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"givenName","James");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"familyName","Dont");
	gcal_contact_set_structured_entry(contact->structured_name,0,1,"nameSuffix","Jr.");

	structured_entry = gcal_contact_get_structured_address(contact);
	gcal_contact_delete_structured_entry(structured_entry,gcal_contact_get_structured_address_count_obj(contact),gcal_contact_get_structured_address_type_obj(contact));

	contact->structured_address->field_typenr = 0;
	contact->structured_address->field_key = malloc(sizeof(char*));
	contact->structured_address->field_key = NULL;
	contact->structured_address->field_value = malloc(sizeof(char*));
	contact->structured_address->field_value = NULL;
	contact->structured_address_nr = 0;
	contact->structured_address_type = (char**)malloc(sizeof(char*));
	address_nr = gcal_contact_set_structured_address_nr(contact,A_HOME);
	address_count = contact->structured_address_nr;
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"street","New Av St, n 61");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"pobox","PO BOX 987 654");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"city","My New Town");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"region","Hereorthere");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"postcode","XYZ 987654-V");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"country","Italy");
	address_nr = gcal_contact_set_structured_address_nr(contact,A_WORK);
	address_count = contact->structured_address_nr;
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"street","Longlong St, n 23");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"pobox","PO BOX 1");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"city","Smallville");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"region","Nowhere");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"postcode","QQQ 112233-WW");
	gcal_contact_set_structured_entry(contact->structured_address,address_nr,address_count,"country","South Africa");

	/* Update contact */
	result = gcal_update_contact(gcal, contact);
	fail_if(result == -1, "Failed editing contact!");

	/* Retrieve updated contacts */
	result = gcal_get_updated_contacts(gcal, &contact_array, NULL);
	fail_if(result == -1, "Failed downloading updated contacts!");

	result = -1;
	for (i = 0; i < contact_array.length; ++i) {
		contact_temp = gcal_contact_element(&contact_array, i);
		if( !strcmp("Dr. James W. Dont Jr.",
			gcal_contact_get_structured_entry(contact_temp->structured_name,0,1,"fullName")) ) {

			temp = gcal_contact_get_nickname(contact);
			fail_if(strcmp("The Fox",temp) != 0,
				"Failed setting/getting right nickname: ---%s---!",temp);

			temp = gcal_contact_get_occupation(contact);
			fail_if(strcmp("Programmer",temp) != 0,
				"Failed setting/getting right occupation: ---%s---!",temp);

			temp = gcal_contact_get_birthday(contact);
			fail_if(strcmp("1963-11-11",temp) != 0,
				"Failed setting/getting right birthday: ---%s---!",temp);

			temp = gcal_contact_get_im_address(contact,1);
			fail_if(strcmp("johnny_aim",temp) != 0,
				"Failed setting/getting right im address: ---%s---!",temp);

			temp = gcal_contact_get_im_protocol(contact,1);
			fail_if(strcmp("AIM",temp) != 0,
				"Failed setting/getting right im protocol: ---%s---!",temp);

			temp = gcal_contact_get_homepage(contact);
			fail_if(strcmp("www.homegage.com",temp) != 0,
				"Failed setting/getting right homepage: ---%s---!",temp);

			temp = gcal_contact_get_blog(contact);
			fail_if(strcmp("myblog.homegage.com",temp) != 0,
				"Failed setting/getting right blog address: ---%s---!",temp);

			temp = gcal_contact_get_structured_entry(contact_temp->structured_address,0,2,"region");
			fail_if(strcmp("Hereorthere",temp) != 0,
				"Failed setting/getting right region of first address: ---%s---!",temp);

			temp = gcal_contact_get_structured_entry(contact_temp->structured_address,1,2,"postcode");
			fail_if(strcmp("QQQ 112233-WW",temp) != 0,
				"Failed setting/getting right postcode of second address: ---%s---!",temp);
		}
	}

	/* Delete */
	result = gcal_erase_contact(gcal, contact);
	fail_if(result == -1, "Failed deleting contact!");

	/* Cleanup */
	gcal_contact_delete(contact);
	gcal_delete(gcal);
	gcal_cleanup_contacts(&contact_array);

}
END_TEST

TCase *gcal_userapi(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 60;
	tc = tcase_create("gcaluserapi");
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_get_calendar);
	tcase_add_test(tc, test_access_calendar);
	tcase_add_test(tc, test_oper_event_event);
	tcase_add_test(tc, test_query_event_updated);
	tcase_add_test(tc, test_get_contacts);
	tcase_add_test(tc, test_access_contacts);
	tcase_add_test(tc, test_oper_contact);
	tcase_add_test(tc, test_query_contact_updated);
	tcase_add_test(tc, test_contact_photo);
	tcase_add_test(tc, test_url_sanity_calendar);
	tcase_add_test(tc, test_url_sanity_contact);
	tcase_add_test(tc, test_contact_new_fields);

	return tc;
}
