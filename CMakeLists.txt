project("libgcal" C)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH 
	"${CMAKE_SOURCE_DIR}/cmake/modules"
	"${CMAKE_SOURCE_DIR}/cmake/3rd-party-modules"
)

option(ENABLE_TESTS "Build unit tests" ON)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

set(GCAL_SOURCE_DIR src)
set(GCAL_HEADER_DIR inc)

include_directories(
	${GCAL_HEADER_DIR}
        ${CURL_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIR}
)

add_subdirectory(${GCAL_SOURCE_DIR})

if(ENABLE_TESTS)
	add_subdirectory(utests)
endif()
