/*
 * @file   utest_contact.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on May 25 2008
 *
 * @brief  Module for google contacts utests.
 */

#include "utest_contact.h"
#include "gcal.h"
#include "gcontact.h"
#include "gcal_parser.h"
#include <string.h>
#include <stdio.h>

static struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_initialize();
	/* here we set libgcal to handle contacts */
	gcal_set_service(ptr_gcal, GCONTACT);
}

static void teardown(void)
{
	/* and here we clean up */
	gcal_destroy(ptr_gcal);
}


START_TEST (test_gcont_dump)
{
	int result;
	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping contacts");

}
END_TEST


START_TEST (test_gcont_authenticate)
{

	int result;

	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
	fail_if(result != 0, "Authentication should work");

	result = gcal_get_authentication("gcal4tester", "fail_fail", ptr_gcal);
	fail_if(result == 0, "Authentication must fail");

}
END_TEST


START_TEST (test_gcont_entries)
{
	/* obs: this test is a copy of utest_gcal.c:test_gcal_entries */
	int result, i;
	char *contacts_emails[] = { "cavalcantii@gmail.com",
				   "gcal4tester@gmail.com",
				   "gcalntester@gmail.com" };
	char *ptr;
	int contacts_count = 3;

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping contacts");

	result = gcal_entries_number(ptr_gcal);
	fail_if(result != 3, "Got wrong number of contacts");

	for (i = 0; i < contacts_count; ++i)
		if (!(ptr = strstr(gcal_access_buffer(ptr_gcal),
				     contacts_emails[i])))
			fail_if(1, "Can't find contact in atom stream. "
				"Position = %i\n", i);


}
END_TEST


START_TEST (test_gcont_extract)
{
	int result;
	size_t count, i;
	struct gcal_contact *contacts;
	char *contacts_email[] = { "cavalcantii@gmail.com",
				   "gcal4tester@gmail.com",
				   "gcalntester@gmail.com" };
	char *contacts_name[] = { "Adenilson Cavalcanti",
				   "", /* its valid not having a name */
				   "gcalntester gcalntester" };

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping contacts");

	contacts = gcal_get_contacts(ptr_gcal, &count);
	fail_if(contacts == NULL, "Failed extracting the contacts vector!");

	if (contacts != NULL)
		for (i = 0; i < count; ++i) {
			fail_if(strcmp(contacts[i].email, contacts_email[i]),
				"extracted data differs from expected: emails");

			fail_if(strcmp(contacts[i].title, contacts_name[i]),
				"extracted data differs from expected: name");

		}


	gcal_destroy_contacts(contacts, count);
}
END_TEST


START_TEST (test_contact_xml)
{
	int result, length;
	struct gcal_contact contact;
	char *xml = NULL, *ptr;


	contact.title = "John Doe";
	contact.email = "john.doe@foo.bar.com";
	contact.id = contact.updated = contact.edit_uri = NULL;
	/* extra fields */
	contact.content = "A very interesting person";
	contact.org_name = "Foo software";
	contact.org_title = "Software engineer";
	contact.im = "john";
	contact.phone_number = "+9977554422119900";
	contact.post_address = "Unknown Av. St., n. 69, Someplace";

	result = xmlcontact_create(&contact, &xml, &length);
	fail_if(result == -1 || xml == NULL,
		"Failed creating XML for a new contact!");

	ptr = strstr(xml, contact.title);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.title);
	ptr = strstr(xml, contact.post_address);
	fail_if(ptr == NULL, "XML lacks a field: %s\n", contact.post_address);

	free(xml);
	fprintf(stderr, "at: %s is nice\n", xml);

}
END_TEST


START_TEST (test_contact_add)
{
	int result;
	struct gcal_contact contact;

	contact.title = "John Doe";
	contact.email = "john.doe@foo.bar.com";
	contact.id = contact.updated = contact.edit_uri = NULL;
	/* extra fields */
	contact.content = "A very interesting person";
	contact.org_name = "Foo software";
	contact.org_title = "Software engineer";
	contact.im = "john";
	contact.phone_number = "+9977554422119900";
	contact.post_address = "Unknown Av. St., n. 69, Someplace";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_contact(&contact, ptr_gcal);
	fail_if(result == -1, "Failed creating a new contact!");


}
END_TEST


TCase *gcontact_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 30;
	tc = tcase_create("gcontact");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_gcont_authenticate);
	tcase_add_test(tc, test_gcont_dump);
	tcase_add_test(tc, test_gcont_entries);
	tcase_add_test(tc, test_gcont_extract);
	tcase_add_test(tc, test_contact_xml);
	tcase_add_test(tc, test_contact_add);
	return tc;
}

