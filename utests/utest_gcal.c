#include "utest_gcal.h"
#include "gcal.h"
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



TCase *gcal_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 30;
	tc = tcase_create("gcal");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_gcal_authenticate);
	tcase_add_test(tc, test_url_parse);
	tcase_add_test(tc, test_gcal_dump);

	return tc;
}


