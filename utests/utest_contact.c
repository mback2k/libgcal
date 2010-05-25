/*
 * @file   utest_contact.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on May 25 2008
 *
 * @brief  Module for google contacts utests.
 */

#include "utest_contact.h"
#include "gcal.h"
#include "gcont.h"
#include "gcontact.h"
#include "gcal_parser.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_construct(GCONTACT);
}

static void teardown(void)
{
	/* and here we clean up */
	gcal_destroy(ptr_gcal);
}


START_TEST (test_contact_dump)
{
	int result;
	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal, "GData-Version: 3.0");
	fail_if(result != 0, "Failed dumping contacts");

}
END_TEST


START_TEST (test_contact_authenticate)
{

	int result;

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result != 0, "Authentication should work");

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "fail_fail");
	fail_if(result == 0, "Authentication must fail");

}
END_TEST


START_TEST (test_contact_entries)
{
	/* obs: this test is a copy of utest_gcal.c:test_gcal_event */
	int result, i;
	char *contacts_emails[] = { "gcal4tester@gmail.com",
				    "gcalntester@gmail.com",
				    "cavalcantii@gmail.com" };
	char *ptr;
	int contacts_count = 3;

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal, "GData-Version: 3.0");
	fail_if(result != 0, "Failed dumping contacts");

	result = gcal_entry_number(ptr_gcal);
	fail_if(result != 3, "Got wrong number of contacts");

	for (i = 0; i < contacts_count; ++i)
		if (!(ptr = strstr(gcal_access_buffer(ptr_gcal),
				     contacts_emails[i])))
			fail_if(1, "Can't find contact in atom stream. "
				"Position = %i\n", i);


}
END_TEST


START_TEST (test_contact_extract)
{
	int result;
	size_t count, i, j;
	struct gcal_contact *contacts;
	char *contacts_email[] = { "gcal4tester@gmail.com",
				   "gcalntester@gmail.com",
				   "cavalcantii@gmail.com" };
	char *contacts_name[] = { "", /* its valid not having a name */
				  "gcalntester",
				  "Adenilson Cavalcanti" };
	size_t contacts_count = 3, found_count = 0;;

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Authentication should work");

	result = gcal_dump(ptr_gcal, "GData-Version: 3.0");
	fail_if(result != 0, "Failed dumping contacts");

	contacts = gcal_get_all_contacts(ptr_gcal, &count);
	fail_if(contacts == NULL, "Failed extracting the contacts vector!");

	for (i = 0; i < contacts_count; ++i)
		for (j = 0; j < count; ++j)
			if ((!(strcmp(contacts[i].emails_field[0],
				      contacts_email[j]))) &&
			    (!(strcmp(contacts[i].common.title,
				      contacts_name[j]))))
				++found_count;

	fail_if(found_count != contacts_count, "Cannot find all 3 contacts!");
	gcal_destroy_contacts(contacts, count);
}
END_TEST


START_TEST (test_contact_xml)
{
	int result, length;
	struct gcal_contact contact;
	char *xml = NULL, *ptr;


	contact.common.title = "John Doe";
	contact.emails_field = malloc(sizeof(char*));
	contact.emails_field[0] = "john.doe@foo.bar.com";
	contact.emails_nr = 1;
	/* TODO: set etag as NULL in all utests here */
	contact.common.id = contact.common.updated = contact.common.edit_uri = contact.common.etag = NULL;
	contact.photo = contact.photo_data = NULL;
	/* extra fields */
	contact.content = "A very interesting person";
	contact.org_name = "Foo software";
	contact.org_title = "Software engineer";
	contact.phone_numbers_field = malloc(sizeof(char*));
	contact.phone_numbers_field[0] = "+9977554422119900";
	contact.emails_type = malloc(sizeof(char*));
	contact.emails_type[0] = "Home";
	contact.phone_numbers_nr = 1;
	contact.phone_numbers_type = malloc(sizeof(char*));
	contact.phone_numbers_type[0] = "Home";
	contact.groupMembership = malloc(sizeof(char*));
	contact.groupMembership[0] = "http://www.google.com/m8/feeds/groups/gcal4tester%40gmail.com/base/6";
	contact.groupMembership_nr = 1;
	contact.birthday = "1980-10-10";
	contact.im = "john_skype";
	contact.homepage = "www.homegage.com";
	contact.homepage = "myblog.homegage.com";
	gcal_contact_set_structured_address(&contact,"street","Unknown Av St, n 69");
	gcal_contact_set_structured_address(&contact,"pobox","PO BOX 123 456");
	gcal_contact_set_structured_address(&contact,"city","Dirty Old Town");
	gcal_contact_set_structured_address(&contact,"region","Somewhere");
	gcal_contact_set_structured_address(&contact,"postcode","ABC 12345-D");
	gcal_contact_set_structured_address(&contact,"country","Madagascar");

	result = xmlcontact_create(&contact, &xml, &length);
	fail_if(result == -1 || xml == NULL,
		"Failed creating XML for a new contact!");


	ptr = strstr(xml, contact.common.title);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.common.title);
	ptr = strstr(xml, contact.emails_field[0]);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.emails_field[0]);
	ptr = strstr(xml, contact.content);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.content);
	ptr = strstr(xml, contact.org_name);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.org_name);
	ptr = strstr(xml, contact.org_title);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.org_title);
	ptr = strstr(xml, contact.phone_numbers_field[0]);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.phone_numbers_field[0]);
	ptr = strstr(xml, contact.post_address);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.post_address);
	ptr = strstr(xml, contact.birthday);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.birthday);
	/* TODO: im requires a new field for service type (i.e. AIM, yahoo,
	 * skype, etc)
	 */
	ptr = strstr(xml, contact.im);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.im);

	free(xml);

}
END_TEST


START_TEST (test_contact_add)
{
	int result;
	struct gcal_contact contact;

	contact.common.title = "John Doe";
	contact.emails_field = malloc(sizeof(char*));
	contact.emails_field[0] = "john.doe@foo.bar.com";
	contact.emails_type = malloc(sizeof(char*));
	contact.emails_type[0] = "home";
	contact.emails_nr = 1;
	contact.pref_email = 0;
	contact.common.id = contact.common.updated = contact.common.edit_uri = contact.common.etag = NULL;
	contact.photo = contact.photo_data = NULL;
	/* extra fields */
	contact.content = "A very interesting person";
	contact.org_name = "Foo software";
	contact.org_title = "Software engineer";
	contact.im = "john";
	contact.phone_numbers_field = malloc(sizeof(char*));
	contact.phone_numbers_field[0] = "+9977554422119900";
	contact.phone_numbers_type = malloc(sizeof(char*));
	contact.phone_numbers_type[0] = "home";
	contact.phone_numbers_nr = 1;
	contact.groupMembership = malloc(sizeof(char*));
	contact.groupMembership[0] = "http://www.google.com/m8/feeds/groups/gcalntester%40gmail.com/base/6";
	contact.groupMembership_nr = 1;
	contact.birthday = "1980-10-10";
	contact.homepage = "www.homegage.com";
	contact.homepage = "myblog.homegage.com";
	gcal_contact_set_structured_address(&contact,"street","Unknown Av St, n 69");
	gcal_contact_set_structured_address(&contact,"pobox","PO BOX 123 456");
	gcal_contact_set_structured_address(&contact,"city","Dirty Old Town");
	gcal_contact_set_structured_address(&contact,"region","Somewhere");
	gcal_contact_set_structured_address(&contact,"postcode","ABC 12345-D");
	gcal_contact_set_structured_address(&contact,"country","Madagascar");

	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_contact(ptr_gcal, &contact, NULL);
	fail_if(result == -1, "Failed creating a new contact!");

	/* I commented this test because it prints too much error
	 * information to stderr. I must add a logging of some sort
	 * to library soon.
	 */
	/* Trying to insert another contact with same email
	 * should trigger HTTP 409 Conflict code.
	 */
/* 	result = gcal_create_contact(&contact, ptr_gcal); */
/* 	fail_if(result != -1, "Adding the same contact must conflict!"); */


}
END_TEST


START_TEST (test_contact_delete)
{
	/* This test assumes that 'test_contact_add' worked fine:
	 * I'm going to delete that contact.
	 */
	char *title = "John Doe";
	char *email = "john.doe@foo.bar.com";
	struct gcal_contact *contacts;
	int i, result, entry_index = -1;
	size_t count = 0;

	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_dump(ptr_gcal, "GData-Version: 3.0");
	fail_if(result != 0, "Failed dumping contacts");

	contacts = gcal_get_all_contacts(ptr_gcal, &count);
	fail_if(contacts == NULL, "Failed extracting contacts vector!");

	for (i = 0; i < (int)count; ++i)
	    if (contacts[i].emails_field) {
		if ((!(strcmp(contacts[i].emails_field[0], email))) &&
		    (!(strcmp(contacts[i].common.title, title)))) {
			entry_index = i;
			break;
		    }
	    }
	fail_if(entry_index == -1, "Cannot locate the newly added contact!");
/* 	fprintf(stderr, "index = %d\tname = %s\n", entry_index, */
/* 		contacts[entry_index].common.title); */

	result = gcal_delete_contact(ptr_gcal, (contacts + entry_index));
	fail_if(result == -1, "Failed deleting contact!");

	gcal_destroy_contacts(contacts, count);
}
END_TEST


START_TEST (test_contact_edit)
{
	int result;
	struct gcal_contact contact, contact_new, updated;

	contact.common.title = "Johny Doe";
	contact.photo = contact.photo_data = NULL;
	contact.emails_field = malloc(sizeof(char*));
	contact.emails_field[0] = "johny.doe@foo.bar.com";
	contact.emails_nr = 1;
	contact.pref_email = 0;
	contact.emails_type = malloc(sizeof(char*));
	contact.emails_type[0] = "home";
	contact.common.id = contact.common.updated = contact.common.edit_uri = contact.common.etag = NULL;
	/* extra fields */
	contact.content = "A very interesting person";
	contact.org_name = "Foo software";
	contact.org_title = "Software engineer";
	contact.im = "johny";
	contact.phone_numbers_field = malloc(sizeof(char*));
	contact.phone_numbers_field[0] = "+9977554422119900";
	contact.phone_numbers_type= malloc(sizeof(char*));
	contact.phone_numbers_type[0] = "home";
	contact.phone_numbers_nr = 1;
	contact.groupMembership = malloc(sizeof(char*));
	contact.groupMembership[0] = "http://www.google.com/m8/feeds/groups/gcalntester%40gmail.com/base/6";
	contact.groupMembership_nr = 1;
	contact.birthday = "1980-10-10";
	contact.homepage = "www.homegage.com";
	contact.homepage = "myblog.homegage.com";
	gcal_contact_set_structured_address(&contact,"street","Unknown Av St, n 69");
	gcal_contact_set_structured_address(&contact,"pobox","PO BOX 123 456");
	gcal_contact_set_structured_address(&contact,"city","Dirty Old Town");
	gcal_contact_set_structured_address(&contact,"region","Somewhere");
	gcal_contact_set_structured_address(&contact,"postcode","ABC 12345-D");
	gcal_contact_set_structured_address(&contact,"country","Madagascar");

	/* Authenticate and add a new contact */
	result = gcal_get_authentication(ptr_gcal, "gcalntester", "77libgcal");
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_contact(ptr_gcal, &contact, &contact_new);
	fail_if(result == -1, "Failed creating a new contact!");

	/* Edit this guy */
	free(contact_new.common.title);
	contact_new.common.title = strdup("Johny 'the mad' Doe");
	result = gcal_edit_contact(ptr_gcal, &contact_new, &updated);
	fail_if(result == -1, "Failed editing contact!");

	/* Delete the contact: pay attention that each edit changes
	 * the "edit_url" field!
	 */
	result = gcal_delete_contact(ptr_gcal, &updated);
	fail_if(result == -1, "Failed deleting contact!");

	/* Do memory clean up */
	gcal_destroy_contact(&contact_new);
	gcal_destroy_contact(&updated);

}
END_TEST


TCase *gcontact_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;
	tc = tcase_create("gcontact");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_contact_authenticate);
	tcase_add_test(tc, test_contact_dump);
	tcase_add_test(tc, test_contact_entries);
	tcase_add_test(tc, test_contact_extract);
	tcase_add_test(tc, test_contact_xml);
	tcase_add_test(tc, test_contact_add);
	tcase_add_test(tc, test_contact_delete);
	tcase_add_test(tc, test_contact_edit);
	return tc;
}

