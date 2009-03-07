find_package(Check REQUIRED)

enable_testing()

# Try and find GCOV for test coverage.
find_program(GCOV gcov)

include_directories(${CHECK_INCLUDE_DIRS})

set(UTESTS_SRCS
	utest.c
	utest_contact.c
	utest_debug.c
	utest_edit.c
	utest_gcal.c
	utest_query.c
	utest_screw.c
	utest_userapi.c
	utest_xmlmode.c
	utest_xpath.c
	utils.c
)

add_executable(test ${UTESTS_SRCS})
target_link_libraries(test gcal ${CHECK_LIBRARIES})
