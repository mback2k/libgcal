project("libgcal" C)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH 
	"${CMAKE_SOURCE_DIR}/cmake/modules"
	"${CMAKE_SOURCE_DIR}/cmake/3rd-party-modules"
)

set(GCAL_VERSION_MAJOR 0)
set(GCAL_VERSION_MINOR 8)
set(GCAL_VERSION_PATCH 5)
set(GCAL_VERSION 
	${GCAL_VERSION_MAJOR}.${GCAL_VERSION_MINOR}.${GCAL_VERSION_PATCH}
)
set(GCAL_VERSION_COUNT 3)

message(STATUS "Building ${PROJECT_NAME} ${GCAL_VERSION}")

option(ENABLE_TESTS "Build unit tests" ON)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

set(GCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(GCAL_HEADER_DIR "${CMAKE_SOURCE_DIR}/inc")

set(GCAL_LIBRARY_INSTALL_DIR "lib")
set(GCAL_INCLUDE_INSTALL_DIR "include")

include_directories(
	${GCAL_HEADER_DIR}
        ${CURL_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIR}
)

add_subdirectory(${GCAL_SOURCE_DIR})

if(ENABLE_TESTS)
	add_subdirectory(utests)
endif()

# Generate Doxygen Documentation
configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.cmake Doxyfile @ONLY)
add_custom_target(doc doxygen)

# Generate pkg-config files
configure_file(${CMAKE_SOURCE_DIR}/libgcal.pc.cmake libgcal.pc @ONLY)
install(
	FILES "${CMAKE_BINARY_DIR}/libgcal.pc"
	DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}/pkgconfig"
)

# Install header files
install(
	DIRECTORY "${GCAL_HEADER_DIR}/"
        DESTINATION "${GCAL_INCLUDE_INSTALL_DIR}"
	FILES_MATCHING PATTERN "*.h"
)
