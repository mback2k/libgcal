# Copyright (c) 2009 Mike Arthur <mike@mikearthur.co.uk>

set(GCAL_SOURCE_FILES
	atom_parser.c
	gcal.c
	gcalendar.c
	gcal_parser.c
	gcal_status.c
	gcontact.c
	gcont.c
	xml_aux.c
)

if(CURL_DEBUG)
	set(GCAL_SOURCE_FILES ${GCAL_SOURCE_FILES} curl_debug_gcal.c)
endif()

add_library(gcal SHARED ${GCAL_SOURCE_FILES})
target_link_libraries(gcal ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})

install(
	TARGETS gcal
	LIBRARY DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}"
	ARCHIVE DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}"
)
