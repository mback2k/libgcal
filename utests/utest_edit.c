/*
 * @file   utest_edit.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Tue Apr  8 09:09:25 2008
 *
 * @brief  Module for tests for edit/add/delete
 */

#include "utest_edit.h"
#include "gcal.h"
#include "xml_aux.h"
#include "gcal_parser.h"
#include <string.h>

static struct gcal_resource *ptr_gcal = NULL;

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

START_TEST (test_edit_xmlres)
{
	xmlTextWriter *writer;
	xmlBuffer *buffer;
	int result;

	result = xmlentry_init_resources(&writer, &buffer);
	fail_if(result == -1, "Failed creating XML resources");

	xmlentry_destroy_resources(&writer, &buffer);


}
END_TEST

START_TEST (test_edit_xml)
{
	struct gcal_entries event;
	char *xml = NULL;
	int result, length;

	event.title = "A new event";
	event.content = "Here goes the description of my new event";
	event.dt_start = "2008-04-08T08:00:00.000Z";
	event.dt_end = "2008-04-08T09:00:00.000Z";
	event.where = "someplace";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = xmlentry_create(&event, &xml, &length);
	fail_if(result == -1 || xml == NULL,
		"Failed creating XML for a new calendar entry");

	/* TODO: add a way to validate the generated XML. */

	free(xml);

}
END_TEST

START_TEST (test_edit_add)
{
	int result;
	struct gcal_entries event;

	event.title = "A new event";
	event.content = "Here goes the description of my new event";
	event.dt_start = "2008-04-08T08:00:00.000Z";
	event.dt_end = "2008-04-08T09:00:00.000Z";
	event.where = "someplace";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_event(&event, ptr_gcal);
	fail_if(result = -1, "Failed creating a new event!");


}
END_TEST

TCase *edit_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 90;
	tc = tcase_create("edit");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_edit_add);
	tcase_add_test(tc, test_edit_xmlres);
	tcase_add_test(tc, test_edit_xml);
	return tc;
}


