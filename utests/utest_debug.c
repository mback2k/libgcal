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
#include "utest_debug.h"
#include "gcal.h"
#include "gcontact.h"
#include "gcal_status.h"

static struct gcal_resource *ptr_gcal = NULL;

static void setup(void)
{
	/* here goes any common data allocation */
	ptr_gcal = gcal_initialize(GCONTACT);
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
	fail_if(result != 0, "Authentication should work");

	code = gcal_status_httpcode(ptr_gcal);
	fail_if(code != 200, "Reported HTTP code should be 200!");

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

	return tc;
}
