project("libgcal" C)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH 
	"${CMAKE_SOURCE_DIR}/cmake/modules"
	"${CMAKE_SOURCE_DIR}/cmake/3rd-party-modules"
)

option(ENABLE_TESTS "Build and run unit tests" ON)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

include_directories(
	inc
        ${CURL_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIR}
)

add_subdirectory(src)

if(ENABLE_TESTS)
	add_subdirectory(utests)
endif()
