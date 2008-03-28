/**
 * @file   atom_parser.h
 * @author teste
 * @date   Fri Mar 28 08:07:43 2008
 *
 * @brief  This is the Atom XML feed parser, it provides functions
 * to get the number of events, extract data from those events and
 * so on.
 *
 * It depends on libxml2.
 *
 */

#ifndef __GCAL_ATOM__
#define __GCAL_ATOM__

#include <libxml/parser.h>

/** This function returns the number of event entries that a Atom feed
 * has.
 *
 * @param document Pointer to a pointer of libxml document, if it is NULL,
 * the function will create it and you can reuse it later (just remember to
 * free it using 'xmlFreeDoc')
 *
 * @param xml_data A pointer to string with the Atom stream.
 *
 * @return -1 on error, the number of entries otherwise (can be 0 zero).
 *
 */
int atom_entries(xmlDoc **document, char *xml_data);


#endif
