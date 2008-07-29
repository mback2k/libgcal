/*
 * @file   utest_gcal.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Started on Mar 03 2008
 *
 * @brief  Module for libgcal utests.
 */

#include "utest_gcal.h"
#include "gcal.h"
#include "gcal_parser.h"
#include "utils.h"
#include <string.h>

struct gcal_resource *ptr_gcal = NULL;

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

START_TEST (test_gcal_authenticate)
{

	int result;
	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result != 0, "Authentication should work");

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "fail_fail");
	fail_if(result == 0, "Authentication must fail");

	/* Calling again, it must free internal pointer with previous
	 * authentication.
	 */
	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result == -1, "Authentication should work");

}
END_TEST

START_TEST (test_url_parse)
{
	char value_url[] = "http://www.google.com/calendar/feeds/default"
		"/owncalendars/full?gsessionid=3ymuQGgqKY1Qz8mk5qUJrw";
	char *url;

	char raw_data[] = "<HTML>\n"
		"<HEAD>\n"
		"<TITLE>Moved Temporarily</TITLE>\n"
		"</HEAD>\n"
		"<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\">\n"
		"<H1>Moved Temporarily</H1>\n"
		"The document has moved"
		"<A HREF=\"http://www.google.com/calendar/feeds/default/owncalendars/full?gsessionid=3ymuQGgqKY1Qz8mk5qUJrw\">here</A>.\n"
		"</BODY>\n"
		"</HTML>\n";

	get_the_url(raw_data, sizeof(raw_data), &url);
	fail_if(url == NULL, "Function failed to get the URL");
	fail_if(strncmp(value_url, url, sizeof(value_url)) != 0,
			"Returned url is wrong");

}
END_TEST

START_TEST (test_editurl_parse)
{
	char *super_contact = NULL;
	char *edit_url = NULL;
	char *tmp;
	int result;
	if (find_load_file("/utests/fullcontact.xml", &super_contact))
		fail_if(1, "Cannot load contact XML file!");

	result = get_edit_url(super_contact, strlen(super_contact), &edit_url);
	fail_if(result == -1, "Failed extracting edit URL from raw XML entry!");
	fail_if(edit_url == NULL, "Failed extracting edit URL from raw XML entry!");

	tmp = strcmp(edit_url, "http://www.google.com/m8/feeds/contacts/gcalntester%40gmail.com/base/a1fa2ca095c082e/1216490120006000");
	fail_if(tmp != 0, "Extracted URL differs from sample file!");
	free(super_contact);
	free(edit_url);

	/* TODO: load_file is failing, I will need to fix that later. */
/* 	if (find_load_file("/utests/fullcalendar.xml", &super_contact)) */
/* 		fail_if(1, "Cannot load calendar XML file!"); */
/* 	fprintf(stderr, "XML: %s nice!\n", super_contact); */

/* 	result = get_edit_url(super_contact, strlen(super_contact), &edit_url); */
/* 	fail_if(result == -1, "Failed extracting edit URL from raw XML entry!"); */
/* 	fail_if(edit_url == NULL, "Failed extracting edit URL from raw XML entry!"); */
/* 	tmp = strstr(edit_url, "http://www.google.com"); */
/* 	fail_if(tmp == NULL, "Cannot find address, check if URL is correct!"); */
/* 	fprintf(stderr, "url: %s is nice!\n", edit_url); */
/* 	free(super_contact); */
/* 	free(edit_url); */

/* 	if (find_load_file("/utests/supercontact.xml", &super_contact)) */
/* 		fail_if(1, "Cannot load contact XML file!"); */
/* 	result = get_edit_url(super_contact, strlen(super_contact), &edit_url); */
/* 	fail_if(edit_url != NULL, "This file has no edit URL. Failed!"); */
/* 	free(super_contact); */
/* 	free(edit_url); */

}
END_TEST


START_TEST (test_gcal_dump)
{
	int result;
	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping events");

}
END_TEST


START_TEST (test_gcal_event)
{
	int result, i;
	struct gcal_event *entries;
	char *entries_update[] = { "2008-03-26T20:20:51.000Z",
				   "2008-03-26T12:30:06.000Z",
				   "2008-03-10T12:56:43.000Z",
				   "2008-03-06T15:32:25.000Z" };

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping events");

	result = gcal_entry_number(ptr_gcal);
	fail_if(result != 4, "Got wrong number of entries");

	entries = gcal_get_entries(ptr_gcal, &result);
	fail_if(entries == NULL, "Failed extracting the entries vector");

	if (entries != NULL)
		for (i = 0; i < result; ++i)
			fail_if(strcmp(entries[i].common.updated, entries_update[i]),
				"extracted data differs from expected");
	/* Cleanup */
	gcal_destroy_entries(entries, result);
}
END_TEST



START_TEST (test_gcal_naive)
{
	/* This test uses a user/password invalid. The purpose is to check
	 * if the library will behave correctly.
	 */
	size_t result, i;
	struct gcal_resource *local_gcal;
	struct gcal_event *entries;

	local_gcal = gcal_construct(GCALENDAR);
	result = gcal_get_authentication(local_gcal, "username", "a_password");
	fail_if((signed)result != -1, "Authentication must fail!");

	result = gcal_dump(local_gcal);
	fail_if((signed)result != -1, "Dump must fail!");

	entries = gcal_get_entries(local_gcal, &result);
	fail_if(entries, "Getting the calendar field data must fail!");

	if (entries)
		for (i = 0; i < result; ++i)
			printf("%s\t%s\n", entries[i].common.title,
			       entries[i].common.updated);
	/* Cleanup */
	gcal_destroy_entries(entries, result);
	gcal_destroy(local_gcal);

}
END_TEST



TCase *gcal_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 90;
	tc = tcase_create("gcal");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_gcal_authenticate);
	tcase_add_test(tc, test_url_parse);
	tcase_add_test(tc, test_gcal_dump);
	tcase_add_test(tc, test_gcal_event);
	tcase_add_test(tc, test_gcal_naive);
	tcase_add_test(tc, test_editurl_parse);
	return tc;
}


