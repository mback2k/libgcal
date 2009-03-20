project("libgcal" C)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH 
	"${CMAKE_SOURCE_DIR}/cmake/modules"
	"${CMAKE_SOURCE_DIR}/cmake/3rd-party-modules"
)

include(SetBuildType)

set(GCAL_VERSION_MAJOR 0)
set(GCAL_VERSION_MINOR 8)
set(GCAL_VERSION_PATCH 5)
set(GCAL_VERSION 
	${GCAL_VERSION_MAJOR}.${GCAL_VERSION_MINOR}.${GCAL_VERSION_PATCH}
)
set(GCAL_VERSION_COUNT 3)

message(STATUS "Building ${PROJECT_NAME} ${GCAL_VERSION}")

option(ENABLE_TESTS "Build unit tests" ON)
option(CURL_DEBUG "Enable cURL debugging" OFF)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

find_program(CTAGS ctags)

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

set(GCAL_LIBRARY_INSTALL_DIR "lib")
set(GCAL_INCLUDE_INSTALL_DIR "include")
set(GCAL_CMAKE_MODULE_INSTALL_DIR "share/cmake-2.6/Modules")

include(SetAutotoolsVariables)

# Generate Doxygen Documentation
configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in Doxyfile @ONLY)
add_custom_target(doc)
add_custom_target(docs)
add_dependencies(docs doc)
add_custom_target(doxygen doxygen)
add_dependencies(doc doxygen)

# Generate pkg-config files
configure_file(${CMAKE_SOURCE_DIR}/libgcal.pc.in libgcal.pc @ONLY)
install(
	FILES "${CMAKE_BINARY_DIR}/libgcal.pc"
	DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}/pkgconfig"
)

if(CTAGS)
	add_custom_target(tags
		COMMAND ${CTAGS} ${GCAL_SOURCE_DIR}/* ${GCAL_HEADER_DIR}/* ${GCAL_TEST_DIR}/*
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

# Install CMake module
install(
	FILES "${CMAKE_SOURCE_DIR}/cmake/modules/FindLibGCal.cmake"
	DESTINATION "${GCAL_CMAKE_MODULE_INSTALL_DIR}"
)
