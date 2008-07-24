/*
 * @file   utest_debug.c
 * @author Adenilson Cavalcanti
 * @date   Fri Jun 13 14:53:21 2008
 *
 * @brief  Module for libgcal debug functions.
 *
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "utest_debug.h"
#include "gcal.h"
#include "gcontact.h"
#include "gcal_status.h"
#include "utils.h"

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

START_TEST (test_debug_authenticate)
{

	int result, code;

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "66libgcal");
	fail_if(result != 0, "Authentication should work!");

	code = gcal_status_httpcode(ptr_gcal);
	fail_if(code != 200, "Reported HTTP code should be 200!");
	fail_if(gcal_status_msg(ptr_gcal) != NULL, "There should be no msg!");

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "failfail");
	fail_if(result == 0, "Authentication must fail!");

	code = gcal_status_httpcode(ptr_gcal);
	fail_if(code != 403, "Reported HTTP code should be 403!");

}
END_TEST

START_TEST (test_debug_logfile)
{
	int result, length;
	char *file_path = "/tmp/libgcal.log";
	char *error_line = "code: 403";
	char *file_content = NULL;
	int fd;

	result = gcal_status_setlog(ptr_gcal, file_path);
	fail_if(result == -1, "Failed setting log file!");

	result = gcal_get_authentication(ptr_gcal, "gcal4tester", "failfail");
	fail_if(result == 0, "Authentication must fail!");

	/* This triggers the file closing */
	gcal_destroy(ptr_gcal);
	ptr_gcal = NULL;

	/* Read file and check for error msg */
	fd = open(file_path, O_RDONLY);
	fail_if(fd == -1, "Cannot open log file!");
	result = read_file(fd, &file_content, &length);
	fail_if(result, "Failed reading log file!");

	fail_if((!(strstr(file_content, error_line))), "Cannot find HTTP error"
		" message!");

	free(file_content);
	close(fd);
}
END_TEST


TCase *gcaldebug_tcase_create(void)
{
	TCase *tc = NULL;
	int timeout_seconds = 50;

	tc = tcase_create("gcaldebug");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout (tc, timeout_seconds);

	tcase_add_test(tc, test_debug_authenticate);
	tcase_add_test(tc, test_debug_logfile);

	return tc;
}
