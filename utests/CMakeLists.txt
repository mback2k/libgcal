find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

set(GCAL_TEST_SOURCE_FILES
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

add_executable(testgcal ${GCAL_TEST_SOURCE_FILES})
target_link_libraries(testgcal gcal ${CHECK_LIBRARIES})

add_custom_target(
	test
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/testgcal 
	DEPENDS testgcal
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(GCOV)
	add_custom_target(
		coverage
		COMMAND ${GCOV} utest.c -o "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/testgcal.dir/utest.c.gcno"
		DEPENDS test
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	)
endif()
