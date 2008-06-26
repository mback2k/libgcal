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
#include "utest_query.h"
#include "utest_userapi.h"

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
		else if (!(strcmp(test_var, "query")))
			suite_add_tcase(s, gcal_query_tcase_create());
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
	suite_add_tcase(s, gcal_query_tcase_create());
exit:
	return s;
}


static Suite *user_api(void)
{
	Suite *s;
	s = suite_create("userapi");
	suite_add_tcase(s, gcal_userapi());
	return s;
}

int main(void)
{
	int number_failed;
	Suite *s, *sapi;
	SRunner *core, *userapi;

	s = core_suite();
	core = srunner_create(s);
	srunner_run_all(core, CK_VERBOSE);
	number_failed = srunner_ntests_failed(core);
	srunner_free(core);

	sapi = user_api();
	userapi = srunner_create(sapi);
	/* I will not fork the userapi, since I need to save a variable
	 * between each running test.
	 */
	srunner_set_fork_status(userapi, CK_NOFORK);
	srunner_run_all(userapi, CK_VERBOSE);
	number_failed = srunner_ntests_failed(userapi);
	srunner_free(userapi);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
