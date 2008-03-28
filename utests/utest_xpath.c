#define _GNU_SOURCE

#include "utest_xpath.h"
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


}
END_TEST

TCase *xpath_tcase_create(void)
{
	TCase *tc = NULL;
	tc = tcase_create("xpath");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_entry_list);

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

