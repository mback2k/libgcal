find_package(Check REQUIRED)

# Try and find GCOV for test coverage.
find_program(GCOV gcov)

include_directories(${CHECK_INCLUDE_DIRS})

set(TESTGCAL_SRCS
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

add_executable(testgcal ${TESTGCAL_SRCS})
target_link_libraries(testgcal gcal ${CHECK_LIBRARIES})

add_custom_target(
	test
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/testgcal 
	DEPENDS testgcal
)
