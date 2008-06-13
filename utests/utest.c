/**
 * @file   utest.c
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 20:14:13 2008
 *
 * @brief  Unit test module.
 *
 * All units goes here (they are useful to help refine the API of library
 * and also provide a way to validate/document the code).
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "utest_gcal.h"
#include "utest_xpath.h"
#include "utest_edit.h"
#include "utest_contact.h"
#include "utest_debug.h"

static Suite *core_suite(void)
{
	char *test_var;
	Suite *s;

	s = suite_create("core");
	test_var = getenv("GCALTEST");
	if (test_var) {
		if (!(strcmp(test_var, "gcal")))
			suite_add_tcase(s, gcal_tcase_create());
		else if (!(strcmp(test_var, "xpath")))
			suite_add_tcase(s, xpath_tcase_create());
		else if (!(strcmp(test_var, "edit")))
			suite_add_tcase(s, edit_tcase_create());
		else if (!(strcmp(test_var, "contact")))
			suite_add_tcase(s, gcontact_tcase_create());
		else if (!(strcmp(test_var, "debug")))
			suite_add_tcase(s, gcaldebug_tcase_create());
		else
			goto all;

		goto exit;
	}

all:
	suite_add_tcase(s, gcal_tcase_create());
	suite_add_tcase(s, xpath_tcase_create());
	suite_add_tcase(s, edit_tcase_create());
	suite_add_tcase(s, gcontact_tcase_create());
	suite_add_tcase(s, gcaldebug_tcase_create());
exit:
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
