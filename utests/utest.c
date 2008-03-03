#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <stdlib.h>
#include "gcal.h"

static void setup(void)
{
	/* here goes any common data allocation */
}

static void teardown(void)
{
	/* and here we clean up */
}


START_TEST (test_gcal_authenticate)
{

	int result;
	result = gcal_get_authentication("gcal4tester", "66libgcal", NULL);
	fail_if(result != 0, "Failed authentication");

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

	return tc;
}



static Suite *core_suite(void)
{
	Suite *s = suite_create("core");
	suite_add_tcase(s, gcal_tcase_create());
	return s;
}

int main(void)
{
	int number_failed;
	Suite *s = core_suite();
	SRunner *sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);

	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
