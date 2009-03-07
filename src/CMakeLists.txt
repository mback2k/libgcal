set(GCAL_SRCS
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

add_library(gcal SHARED ${GCAL_SRCS})
target_link_libraries(gcal ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})
