#define _GNU_SOURCE
/*
 * @file   utest_gcal.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Thu Mar 27 2008
 *
 * @brief  Header module for xpath utests.
 *
 */

#include "utest_xpath.h"
#include "atom_parser.h"
#include "xml_aux.h"
#include "gcal.h"
#include "internal_gcal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"

char *xml_data = NULL;

static void setup(void)
{
	if (find_load_file("/utests/4entries_location.xml", &xml_data))
		exit(1);
}

static void teardown(void)
{
	/* and here we clean up */
	if (xml_data)
		free(xml_data);
}


START_TEST (test_entry_list)
{

	int num_entries, res;
	xmlDoc *doc = NULL;

	num_entries = atom_entries(doc);
	fail_if(num_entries != -1, "Function tried to proceed with NULL doc!");

	res = build_doc_tree(&doc, xml_data);
	fail_if(res == -1, "failed to build document tree!");

	num_entries = atom_entries(doc);
	fail_if(num_entries != 4, "failed get correct number of entries: "
		"4 != %d\n", num_entries);

	clean_doc_tree(&doc);
}
END_TEST



START_TEST (test_get_entries)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_event known_value;
	struct gcal_event extracted;
	int res;

	gcal_init_event(&known_value);
	gcal_init_event(&extracted);

	res = build_doc_tree(&doc, xml_data);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	fail_if(nodes->nodeNr != 4, "should return 4 entries!");

	res = atom_extract_data(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	known_value.common.title = "an event with location";
	known_value.common.id  = "http://www.google.com/calendar/feeds/gcal4tester%40gmail.com/private/full/saq81ktu4iqv7r20b8ctv70q7s";
	known_value.common.edit_uri  = "http://www.google.com/calendar/feeds/gcal4tester%40gmail.com/private/full/saq81ktu4iqv7r20b8ctv70q7s/63342246051";
	known_value.content  = "I should be there";
	/* The event is not recurrent: for empty fields, I use a empty string */
	known_value.dt_recurrent  = "";
	known_value.dt_start  = "2008-03-26T18:00:00.000-05:00";
	known_value.dt_end = "2008-03-26T19:00:00.000-05:00";
	known_value.where = "my house";
	known_value.status = "http://schemas.google.com/g/2005#event.confirmed";
	known_value.common.updated = "2008-03-26T20:20:51.000Z";

	fail_if(strcmp(known_value.common.title, extracted.common.title),
		"failed field extraction");
	fail_if(strcmp(known_value.common.id, extracted.common.id),
		"failed field extraction");
	fail_if(strcmp(known_value.common.edit_uri, extracted.common.edit_uri),
		"failed field extraction");
	fail_if(strcmp(known_value.content, extracted.content),
		"failed field extraction");
	fail_if(strcmp(known_value.dt_recurrent, extracted.dt_recurrent),
		"failed field extraction");
	fail_if(strcmp(known_value.dt_start, extracted.dt_start),
		"failed field extraction");
	fail_if(strcmp(known_value.dt_end, extracted.dt_end),
		"failed field extraction");
	fail_if(strcmp(known_value.where, extracted.where),
		"failed field extraction");
	fail_if(strcmp(known_value.status, extracted.status),
		"failed field extraction");
	fail_if(strcmp(known_value.common.updated, extracted.common.updated),
		"failed field extraction");

	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_entry(&extracted);
	clean_doc_tree(&doc);
}
END_TEST

START_TEST (test_get_recurrence)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	char recurrence_str[] = "DTSTART;TZID=America/Manaus:20080618T143000";
	struct gcal_event extracted;
	char *file_contents = NULL;
	int res;

	gcal_init_event(&extracted);

	if (find_load_file("/utests/3entries_recurrence.xml", &file_contents))
		fail_if(1, "Cannot load test XML file!");

	res = build_doc_tree(&doc, file_contents);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	res = atom_extract_data(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	fail_if(!strstr(extracted.dt_recurrent, recurrence_str),
		"failed recurrence field extraction!");

	free(file_contents);
	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_entry(&extracted);
	clean_doc_tree(&doc);

}
END_TEST

START_TEST (test_get_event_deleted)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_event extracted;
	char *file_contents = NULL;
	int res;

	gcal_init_event(&extracted);

	if (find_load_file("/utests/up_deleted_event.xml", &file_contents))
		fail_if(1, "Cannot load test XML file!");

	res = build_doc_tree(&doc, file_contents);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	res = atom_extract_data(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	fail_if(extracted.common.deleted != 1,
		"failed parsing deleted event field!");

	free(file_contents);
	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_entry(&extracted);
	clean_doc_tree(&doc);

}
END_TEST

START_TEST (test_get_contact_deleted)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_contact extracted;
	char *file_contents = NULL;
	int res;

	gcal_init_contact(&extracted);

	if (find_load_file("/utests/up_new_delete_contact.xml", &file_contents))
		fail_if(1, "Cannot load test XML file!");

	res = build_doc_tree(&doc, file_contents);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	res = atom_extract_contact(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	fail_if(extracted.common.deleted != 1,
		"failed parsing deleted contact field!");

	free(file_contents);
	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_contact(&extracted);
	clean_doc_tree(&doc);

}
END_TEST

START_TEST (test_get_contact_nophoto)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_contact extracted;
	char *file_contents = NULL;
	int res;

	gcal_init_contact(&extracted);

	if (find_load_file("/utests/empty_photo.xml", &file_contents))
		fail_if(1, "Cannot load test XML file!");

	res = build_doc_tree(&doc, file_contents);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	res = atom_extract_contact(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	fail_if(extracted.photo_length != 0,
		"this contact was supposed to have no photo!");

	fail_if(strcmp(extracted.photo, "http://www.google.com/m8/feeds/photos"
		       "/media/gcalntester%40gmail.com/b4d61ee8bdbf314") != 0,
		"wrong photo url!");

	free(file_contents);
	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_contact(&extracted);
	clean_doc_tree(&doc);
}
END_TEST

START_TEST (test_get_contact_photo)
{
	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_contact extracted;
	char *file_contents = NULL;
	int res;

	gcal_init_contact(&extracted);

	if (find_load_file("/utests/with_photo.xml", &file_contents))
		fail_if(1, "Cannot load test XML file!");

	res = build_doc_tree(&doc, file_contents);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	res = atom_extract_contact(nodes->nodeTab[0], &extracted);
	fail_if(res == -1, "failed to extract data from node!");

	fail_if(extracted.photo_length == 0,
		"this contact was supposed to have a photo!");

	fail_if(strcmp(extracted.photo, "http://www.google.com/m8/feeds/photos"
		       "/media/gcalntester%40gmail.com/1bd255c2889042a7") != 0,
		"wrong photo url!");

	free(file_contents);
	if (xpath_obj)
		xmlXPathFreeObject(xpath_obj);

	gcal_destroy_contact(&extracted);
	clean_doc_tree(&doc);
}
END_TEST


TCase *xpath_tcase_create(void)
{
	TCase *tc = NULL;
	tc = tcase_create("xpath");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_entry_list);
	tcase_add_test(tc, test_get_entries);
	tcase_add_test(tc, test_get_recurrence);
	tcase_add_test(tc, test_get_event_deleted);
	tcase_add_test(tc, test_get_contact_deleted);
	tcase_add_test(tc, test_get_contact_nophoto);
	tcase_add_test(tc, test_get_contact_photo);
	return tc;

}
