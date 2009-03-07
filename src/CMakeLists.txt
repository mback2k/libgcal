set(GCAL_SOURCE_FILES
	atom_parser.c
	curl_debug_gcal.c
	gcal.c
	gcalendar.c
	gcal_parser.c
	gcal_status.c
	gcontact.c
	gcont.c
	xml_aux.c
)

add_library(gcal SHARED ${GCAL_SOURCE_FILES})
target_link_libraries(gcal ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})

install(
	TARGETS gcal
	LIBRARY DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}"
	ARCHIVE DESTINATION "${GCAL_LIBRARY_INSTALL_DIR}"
)
