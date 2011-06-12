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
#include "utest_xmlmode.h"
#include "utest_screw.h"

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

static Suite *xml_api(void)
{
	Suite *s;
	s = suite_create("xmlapi");
	suite_add_tcase(s, xmlmode_tcase_create());
	return s;
}


static Suite *screw_api(void)
{
	Suite *s;
	s = suite_create("screwapi");
	suite_add_tcase(s, gcal_screw());
	return s;

}

static void run_and_clean(SRunner *runner)
{
	/* TODO: use it to determine exit status */
	int number_failed;
	if (!runner)
		return;

	srunner_run_all(runner, CK_VERBOSE);
	number_failed = srunner_ntests_failed(runner);
	srunner_free(runner);
}


int main(void)
{
	char *env_var;
	Suite *s, *sapi, *sxml, *s_screw;
	SRunner *core, *userapi, *xmlapi, *screwtest;

	s = sapi = sxml = s_screw = NULL;
	core = userapi = xmlapi = screwtest = NULL;

	env_var = getenv("GCAL");
	if (env_var) {
		if (!strcmp("core", env_var)) {
			/* Core internals */
			s = core_suite();
			core = srunner_create(s);
		} else if(!strcmp("user", env_var)) {
			sapi = user_api();
			userapi = srunner_create(sapi);
			/* I will not fork the userapi, since I need to
			 * save a variable
			 * between each running test.
			 */
			srunner_set_fork_status(userapi, CK_NOFORK);
		} else if (!strcmp("xml", env_var)) {
			/* This one tests for XML api mode */
			sxml = xml_api();
			xmlapi = srunner_create(sxml);
		} else if (!strcmp("screw", env_var)) {
			/* Tests trying to break libgcal */
			s_screw = screw_api();
			screwtest = srunner_create(s_screw);
		}
	} else { /* Run all tests */
		s = core_suite();
		core = srunner_create(s);

		sapi = user_api();
		userapi = srunner_create(sapi);
		srunner_set_fork_status(userapi, CK_NOFORK);

		sxml = xml_api();
		xmlapi = srunner_create(sxml);

		s_screw = screw_api();
		screwtest = srunner_create(s_screw);
	}

	run_and_clean(core);
	run_and_clean(userapi);
	run_and_clean(xmlapi);
	run_and_clean(screwtest);

	/* TODO: use number of failures to decide exit status, it can
	 * be either EXIT_FAILURE or ...
	 */
	return EXIT_SUCCESS;
}
