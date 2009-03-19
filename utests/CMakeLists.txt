find_package(Check REQUIRED)
find_program(LCOV lcov)
find_program(LCOV_GENHTML genhtml)

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
	COMMAND "${CMAKE_CURRENT_BINARY_DIR}/testgcal" 
	DEPENDS testgcal
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if(GCOV)
	add_custom_target(
		gcov
		COMMAND ${GCOV} utest.c --object-directory "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/testgcal.dir/utest.c.gcno"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
	add_dependencies(gcov test)
endif()

if(LCOV)
	add_custom_target(
		lcov_capture
		COMMAND ${LCOV} --capture --directory "src/" --output-file "${CMAKE_CURRENT_BINARY_DIR}/testgcal.lcov"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
	add_dependencies(lcov_capture test)
	add_custom_target(
		lcov_remove
		COMMAND ${LCOV} --remove "${CMAKE_CURRENT_BINARY_DIR}/testgcal.lcov" '*.h' --output-file "${CMAKE_CURRENT_BINARY_DIR}/testgcal.lcov"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
	add_dependencies(lcov_remove lcov_capture)
	if (LCOV_GENHTML)
		add_custom_target(
			lcov
			COMMAND ${LCOV_GENHTML} "${CMAKE_CURRENT_BINARY_DIR}/testgcal.lcov" --output-directory "${CMAKE_BINARY_DIR}/docs/lcov"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		)
		add_dependencies(lcov lcov_capture)
		add_dependencies(lcov lcov_remove)
		add_dependencies(doc lcov)
	endif()
endif()
