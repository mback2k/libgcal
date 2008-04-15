#ifndef __GCAL_PARSER__
#define __GCAL_PARSER__
/**
 * @file   gcal_parser.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar 31 11:17:02 2008
 *
 * @brief  A thin layer over \ref atom_parser.h, so I can plug another
 * XML parser to libgcal if required.
 * It creates a DOM document from libgcal atom stream and provides functions
 * wrappers to extract data.
 */

#include <libxml/parser.h>
#include "gcal.h"

/** Abstract type to represent a DOM xml tree (a thin layer over xmlDoc).
 */
typedef xmlDoc dom_document;


/** Parses the returned HTML page and extracts the redirection URL
 * that has the Atom feed.
 *
 * \todo Save the xmlDoc structure for future reuse (maybe within
 * structure \ref gcal_resource).
 *
 * @param data Raw data (the HTML page).
 * @param length Data buffer length.
 * @param url Pointer to the pointer (ouch!) that will receive the URL
 * (you should cleanup its memory). It will point to NULL if there is
 * not a URL in the raw data buffer.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int get_the_url(char *data, int length, char **url);


int get_sessionid(char *data, char *buffer, int length);


/** Builds a DOM tree from a XML string.
 *
 * This is a thin wrapper to \ref build_doc_tree.
 *
 * @param xml_data A pointer to a string with XML content.
 *
 * @return NULL on error, a pointer to a document in sucess.
 */
dom_document *build_dom_document(char *xml_data);


/** Clean up a DOM tree.
 *
 * This is a thin wrapper to \ref clean_doc_tree.
 * @param doc A pointer to a document data type.
 */
void clean_dom_document(dom_document *doc);

/** Return the number of calendar entries in the document.
 *
 * This is a thin wrapper to \ref clean_doc_tree.
 * @param doc A pointer to a document data type.
 *
 * @return -1 on error or the number of entries.
 */
int get_entries_number(dom_document *doc);


/** Receiving a DOM document of the Atom stream, it will extract all the event
 * entries and parse then, storing each entry field in a vector of
 * \ref gcal_entries.
 *
 * It depends on \ref atom_extract_data and \ref atom_get_entries.
 *
 * @param doc A document pointer with the Atom stream.
 *
 * @param data_extract A pointer to a pre-allocated vector \ref gcal_entries.
 *
 * @param length Its length, should be the same as the number of entries. See
 * also \ref get_entries_number.
 *
 * @return 0 on success, -1 on error.
 */
int extract_all_entries(dom_document *doc,
			struct gcal_entries *data_extract, int length);


/** Creates the XML for a new calendar entry.
 *
 * It depends on \ref xmlentry_init_resources and
 * \ref xmlentry_destroy_resources.
 *
 * @param entry A pointer to an calendar entry event (see \ref gcal_entries).
 *
 * @param xml_entry Pointer to pointer string (you must free its memory!).
 *
 * @param length A pointer to a variable that will have its length.
 *
 * @return 0 on sucess, -1 on error.
 */
int xmlentry_create(struct gcal_entries *entry, char **xml_entry, int *length);


#endif
