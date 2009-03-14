# - Set build type automatically if not manually specified.
#
#  Automatically sets the build type to "Debug" unless otherwise set.
#
# This macro defines the following variables:
# CMAKE_DEBUG_BUILD             - Indicates that this is a debug build.
# CMAKE_INSTALL_DEBUG_LIBRARIES - Tells CMake that debug versions of
#                                 prerequisite libraries should be installed.

# Compile in "Debug" mode by default except with MSVC which handles
# Release/Debug builds internally.
if(NOT MSVC OR CMAKE_GENERATOR STREQUAL "NMake Makefiles")
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE
			Debug CACHE 
			STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." 
			FORCE
		)
	else()
		message(STATUS "Compiling in ${CMAKE_BUILD_TYPE} mode")
	endif()
else()
	# Hide the CMAKE_BUILD_TYPE variable when using MSVC.
	mark_as_advanced(FORCE CMAKE_BUILD_TYPE)
endif()

# If we're a debug build, set the necessary variables
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR
   CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(CMAKE_DEBUG_BUILD TRUE)
	set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
