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
#include <unistd.h>

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
	event.dt_start = "2008-05-10T08:00:00.000Z";
	event.dt_end = "2008-05-10T09:00:00.000Z";
	event.where = "someplace";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_event(&event, ptr_gcal);
	fail_if(result == -1, "Failed creating a new event!");


}
END_TEST

START_TEST (test_edit_delete)
{

	int result, i, entry_index = -1;
	struct gcal_entries event;
	struct gcal_entries *entries;

	event.title = "A soon to be deleted event";
	event.content = "This event will be included and deleted soon";
	event.dt_start = "2008-05-07T08:00:00.000Z";
	event.dt_end = "2008-05-07T09:00:00.000Z";
	event.where = "nevermind";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_event(&event, ptr_gcal);
	fail_if(result == -1, "Failed creating a new event!");

	result = gcal_dump(ptr_gcal);
	fail_if(result == -1, "Failed dumping events");

	entries = gcal_get_entries(ptr_gcal, &result);
	fail_if(entries == NULL, "Failed extracting entries");

	for (i = 0; i < result; ++i)
		if (!strcmp(entries[i].title, event.title)) {
			entry_index = i;
			break;
		}
	fail_if(entry_index == -1, "Cannot locate the newly added event!");

	result = gcal_delete_event((entries + entry_index), ptr_gcal);
	fail_if(result == -1, "Failed deleting event!");

	/* Cleanup */
	gcal_destroy_entries(entries, result);
}
END_TEST



START_TEST (test_edit_stress)
{
	int result, i, j, total;
	struct gcal_entries event;
	char *format_start, *format_end;
	char start_buffer[256], end_buffer[256];
	char number_buffer1[4], number_buffer2[4];
	struct gcal_entries *entries;

	event.title = "A new event: stress test";
	event.content = "Here goes the description of my new event";
	format_start = "2008-05-%sT%s:00:00.000Z";
	format_end = "2008-05-%sT%s:00:00.000Z";
	event.where = "someplace";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");


	/* Whole month loop */
	for (i = 1; i < 31; i += 2) {
		/* whole day loop */
		for (j = 4; j < 23; j += 3) {
			snprintf(number_buffer1, sizeof(number_buffer1) - 1,
				 "%02d", i);
			snprintf(number_buffer2, sizeof(number_buffer2) - 1,
				 "%02d", j);
			snprintf(start_buffer, sizeof(start_buffer) - 1,
				 format_start, number_buffer1, number_buffer2);

			snprintf(number_buffer2, sizeof(number_buffer2) - 1,
				 "%02d", j + 1);
			snprintf(end_buffer, sizeof(end_buffer) - 1,
				 format_end, number_buffer1, number_buffer2);

			event.dt_start = start_buffer;
			event.dt_end = end_buffer;
			result = gcal_create_event(&event, ptr_gcal);
			fail_if(result == -1, "Failed creating a new event!"
				" Loop is i = %d\tj = %d", i, j);
		}
	}

	/* Give me some time to check if everything was added
	 * TODO: read an environment variable with the sleep time.
	 */
	sleep(10);

	/* Delete all test events */
	result = gcal_dump(ptr_gcal);
	fail_if(result == -1, "Failed dumping events");

	entries = gcal_get_entries(ptr_gcal, &total);
	fail_if(entries == NULL, "Failed extracting entries");
	for (i = 0; i < total; ++i)
		if (!strcmp(entries[i].title, event.title)) {
			result = gcal_delete_event(&entries[i], ptr_gcal);
			fail_if(result == -1, "Failed deleting event!");
		}


}
END_TEST

START_TEST (test_edit_edit)
{

	int result, i, entry_index = -1;
	struct gcal_entries event;
	struct gcal_entries *entries;

	event.title = "An editable event";
	event.content = "This event will be included and edited";
	event.dt_start = "2008-05-07T08:00:00.000Z";
	event.dt_end = "2008-05-07T09:00:00.000Z";
	event.where = "nevermind";
	/* TODO: think in a better way to describe the status, maybe use
	 * a set of strings.
	 */
	event.status = "confirmed";

	result = gcal_get_authentication("gcalntester", "77libgcal", ptr_gcal);
	fail_if(result == -1, "Authentication should work.");

	result = gcal_create_event(&event, ptr_gcal);
	fail_if(result == -1, "Failed creating a new event!");

	result = gcal_dump(ptr_gcal);
	fail_if(result == -1, "Failed dumping events");

	entries = gcal_get_entries(ptr_gcal, &result);
	fail_if(entries == NULL, "Failed extracting entries");

	for (i = 0; i < result; ++i)
		if (!strcmp(entries[i].title, event.title)) {
			entry_index = i;
			break;
		}
	fail_if(entry_index == -1, "Cannot locate the newly added event!");

	event.title = "An editable event: edited now!";
	result = gcal_edit_event((entries + entry_index), ptr_gcal);
	fail_if(result == -1, "Failed editing event!");

	/* Cleanup */
	gcal_destroy_entries(entries, result);
}
END_TEST

TCase *edit_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 1200;
	tc = tcase_create("edit");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);
	tcase_add_test(tc, test_edit_add);
	tcase_add_test(tc, test_edit_xmlres);
	tcase_add_test(tc, test_edit_xml);
	tcase_add_test(tc, test_edit_delete);
	tcase_add_test(tc, test_edit_stress);
	tcase_add_test(tc, test_edit_edit);
	return tc;
}


