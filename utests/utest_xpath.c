#include "utest_xpath.h"
#include <string.h>


START_TEST (test_entry_list)
{


}
END_TEST

TCase *xpath_tcase_create(void)
{
	TCase *tc = NULL;
	tc = tcase_create("xpath");
	tcase_add_test(tc, test_entry_list);

	return tc;

}
