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
#include <string.h>

struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_initialize();
}

static void teardown(void)
{
	/* and here we clean up */
	gcal_destroy(ptr_gcal);
}

START_TEST (test_gcal_authenticate)
{

	int result;
	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
	fail_if(result != 0, "Authentication should work");

	result = gcal_get_authentication("gcal4tester", "fail_fail", ptr_gcal);
	fail_if(result == 0, "Authentication must fail");

	/* Calling again, it must free internal pointer with previous
	 * authentication.
	 */
	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
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

START_TEST (test_gcal_dump)
{
	int result;
	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping events");

}
END_TEST


START_TEST (test_gcal_entries)
{
	int result, i;
	struct gcal_entries *entries;
	char *entries_update[] = { "2008-03-26T20:20:51.000Z",
				   "2008-03-26T12:30:06.000Z",
				   "2008-03-10T12:56:43.000Z",
				   "2008-03-06T15:32:25.000Z" };

	result = gcal_get_authentication("gcal4tester", "66libgcal", ptr_gcal);
	if (result)
		fail_if(1, "Authentication should work");

	result = gcal_dump(ptr_gcal);
	fail_if(result != 0, "Failed dumping events");

	result = gcal_entries_number(ptr_gcal);
	fail_if(result != 4, "Got wrong number of entries");

	entries = gcal_get_entries(ptr_gcal, &result);
	fail_if(entries == NULL, "Failed extracting the entries vector");

	if (entries != NULL)
		for (i = 0; i < result; ++i)
			fail_if(strcmp(entries[i].updated, entries_update[i]),
				"extracted data differs from expected");

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
	struct gcal_entries *entries;

	local_gcal = gcal_initialize();
	result = gcal_get_authentication("username", "a_password", local_gcal);
	fail_if((signed)result != -1, "Authentication must fail!");

	result = gcal_dump(local_gcal);
	fail_if((signed)result != -1, "Dump must fail!");

	entries = gcal_get_entries(local_gcal, &result);
	fail_if(entries, "Getting the calendar field data must fail!");

	if (entries)
		for (i = 0; i < result; ++i)
			printf("%s\t%s\n", entries[i].title,
			       entries[i].updated);

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
	tcase_add_test(tc, test_gcal_entries);
	tcase_add_test(tc, test_gcal_naive);
	return tc;
}


