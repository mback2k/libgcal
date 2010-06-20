# Copyright (c) 2009 Mike Arthur <mike@mikearthur.co.uk>

project("LibGCal" C)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH
	"${CMAKE_SOURCE_DIR}/cmake/modules"
	"${CMAKE_SOURCE_DIR}/cmake/3rd-party-modules/opensync"
)

include(SetBuildType)

set(GCAL_VERSION_MAJOR 0)
set(GCAL_VERSION_MINOR 9)
set(GCAL_VERSION_PATCH 5)
set(GCAL_VERSION
	${GCAL_VERSION_MAJOR}.${GCAL_VERSION_MINOR}.${GCAL_VERSION_PATCH}
)
set(GCAL_VERSION_COUNT 3)

message(STATUS "Building ${PROJECT_NAME} ${GCAL_VERSION}")

set(LIB_SUFFIX "" CACHE STRING
	"Define suffix of library directory name (e.g. 32/64 for lib32/lib64)"
)

option(ENABLE_TESTS "Build unit tests" ON)
option(CURL_DEBUG "Enable cURL debugging" OFF)
if(CURL_DEBUG)
  ADD_DEFINITIONS(-DGCAL_DEBUG_CURL)
endif()

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

find_program(CTAGS etags)
find_program(DOXYGEN doxygen)

if(CMAKE_DEBUG_BUILD)
	# Try and find GCOV for test coverage.
	find_program(GCOV gcov)
	set(CMAKE_C_FLAGS
		"${CMAKE_C_FLAGS} -Wall -W -Wno-pointer-sign -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual -Wno-pointer-sign"
        )
endif()

set(GCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(GCAL_HEADER_DIR "${CMAKE_SOURCE_DIR}/inc")
set(GCAL_TEST_DIR "${CMAKE_SOURCE_DIR}/utests")

set(GCAL_LIBRARY_INSTALL_DIR "lib${LIB_SUFFIX}")
set(GCAL_INCLUDE_INSTALL_DIR "include")
set(GCAL_CMAKE_MODULE_INSTALL_DIR "${CMAKE_ROOT}/Modules")

include(SetAutotoolsVariables)

add_custom_target(doc)
add_custom_target(docs)
add_dependencies(docs doc)

# Generate Doxygen Documentation
if(DOXYGEN)
	configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in Doxyfile @ONLY)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/docs")
	add_custom_target(doxygen ${DOXYGEN})
	add_dependencies(doc doxygen)
endif()

# Generate pkg-config files
configure_file(${CMAKE_SOURCE_DIR}/libgcal.pc.in ${CMAKE_BINARY_DIR}/libgcal.pc @ONLY)
install(
	FILES "${CMAKE_BINARY_DIR}/libgcal.pc"
	DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}/pkgconfig"
)


# Generate CMake package configuration file
configure_file(${CMAKE_SOURCE_DIR}/cmake/LibGCalConfig.cmake.in ${CMAKE_BINARY_DIR}/cmake/LibGCalConfig.cmake @ONLY)
# TODO: Install this to ${GCAL_LIBRARY_INSTALL_DIR}/cmake/LibGCal when we 
# depend on CMake >= 2.6.3
install(
	FILES "${CMAKE_BINARY_DIR}/cmake/LibGCalConfig.cmake"
	DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}/LibGCal/cmake"
)

if(CTAGS)
	add_custom_target(tags
		COMMAND ${CTAGS} ${GCAL_SOURCE_DIR}/*.c ${GCAL_HEADER_DIR}/*.h ${GCAL_TEST_DIR}/*.?
	)
endif()

include_directories(
	${GCAL_HEADER_DIR}
        ${CURL_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIR}
)

# If we've found GCov then add the necessary profiling flags.
if(GCOV)
	set(CMAKE_C_FLAGS
		"${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage"
	)
endif()

add_subdirectory(${GCAL_SOURCE_DIR})

if(ENABLE_TESTS)
	add_subdirectory(${GCAL_TEST_DIR})
endif()

# Install header files
install(
	DIRECTORY "${GCAL_HEADER_DIR}/"
        DESTINATION "${GCAL_INCLUDE_INSTALL_DIR}"
	FILES_MATCHING PATTERN "*.h"
)

# Add uninstall
configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)
add_custom_target(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)


# Make dist
set(PROJECT_VERSION ${GCAL_VERSION})
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
