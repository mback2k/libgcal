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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *xml_data = NULL;

int read_file(int fd, char **buffer, size_t *length);
char *find_file_path(char *file_name);

static void setup(void)
{
	int fd, res;
	size_t len = 0;
	char *file_name = "/utests/4entries_location.xml";
	char *tmp;

	tmp = find_file_path(file_name);
	fd = open(tmp, O_RDONLY);

	if (fd == -1)
		printf("cannot open file 4entries_location.xml.");

	res = read_file(fd, &xml_data, &len);
	fail_if(res, "failed reading the file!\n");

	free(tmp);
	close(fd);
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
	fail_if(num_entries != 4, "failed get correct number of entries");

	clean_doc_tree(&doc);
}
END_TEST



START_TEST (test_get_entries)
{

	xmlXPathObject *xpath_obj = NULL;
	xmlDoc *doc = NULL;
	xmlNodeSet *nodes;
	struct gcal_entries known_value;
	struct gcal_entries *ptr = NULL;
	int res;

	res = build_doc_tree(&doc, xml_data);
	fail_if(res == -1, "failed to build document tree!");

	xpath_obj = atom_get_entries(doc);
	fail_if(xpath_obj == NULL, "failed to get entry node list!");

	nodes = xpath_obj->nodesetval;
	fail_if(nodes->nodeNr != 4, "should return 4 entries!");

	ptr = atom_extract_data(nodes->nodeTab[0]);
	fail_if(ptr == NULL, "failed to extract data from node!");

	known_value.title = "an event with location";
	known_value.id  = "http://www.google.com/calendar/feeds/gcal4tester%40gmail.com/private/full/saq81ktu4iqv7r20b8ctv70q7s";
	known_value.edit_uri  = "http://www.google.com/calendar/feeds/gcal4tester%40gmail.com/private/full/saq81ktu4iqv7r20b8ctv70q7s/63342246051";
	known_value.content  = "I should be there";
	/* The event is not recurrent */
	known_value.dt_recurrent  = "";
	known_value.dt_start  = "2008-03-26T18:00:00.000-05:00";
	known_value.dt_end = "2008-03-26T19:00:00.000-05:00";
	known_value.where = "my house";
	known_value.status = "http://schemas.google.com/g/2005#event.confirmed";
	known_value.updated = "2008-03-26T20:20:51.000Z";

	/* TODO: put code to test each field here */

}
END_TEST

TCase *xpath_tcase_create(void)
{
	TCase *tc = NULL;
	tc = tcase_create("xpath");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_entry_list);
	tcase_add_test(tc, test_get_entries);
	return tc;

}


int read_file(int fd, char **buffer, size_t *length)
{
	int result = -1, bytes = 0;
	size_t chunk = 256;

	if (!*buffer) {
		*length = chunk;
		*buffer = (char *) malloc(*length);
		if (!buffer)
			goto exit;
	}

	result = read(fd, *buffer, *length);
	while ((result != 0) && (result != -1)) {
		*length += chunk;
		*buffer = realloc(*buffer, *length);
		if (!*buffer) {
			result = -1;
			goto exit;
		}
		bytes += result;
		result = read(fd, (*buffer + bytes), chunk);
	}

	result = 0;

exit:
	return result;

}

char *find_file_path(char *file_name)
{
	char *path, *tmp = NULL;
	int len = 0;
	path = getcwd(NULL, 0);
	if (path == NULL)
		printf("cannot get working directory!");

	/* This should run in a subdirectory 'build' */
	len = strrchr(path, '/') - path;
	tmp = malloc(len + strlen(file_name) + 1);
	strncpy(tmp, path, len);
	strncat(tmp, file_name, len + strlen(file_name) + 1);

	free(path);
	return tmp;

}

