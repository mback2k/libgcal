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
#include <libxml/xpath.h>
#include "gcal.h"

/** Creates a document tree (required by other operations).
 *
 *
 * @param document Creates a XML tree document,  remember to
 * free it using \ref clean_doc_tree)
 * @param xml_data A pointer to string with the Atom stream.
 *
 * @return -1 on error, 0 on success.
 */
int build_doc_tree(xmlDoc **document, char *xml_data);

/** Cleans up a document tree.
 *
 *
 * @param document Document pointer to pointer (it makes it point to NULL).
 */
void clean_doc_tree(xmlDoc **document);


/** This function returns the number of event entries that a Atom feed
 * has.
 *
 * @param document Pointer to a pointer of libxml document.
 *
 * @return -1 on error, the number of entries otherwise (can be 0 zero).
 *
 */
int atom_entries(xmlDoc *document);


/** Get a list of entry nodes from Atom feed.
 *
 *
 * @param document Pointer to a libxml document.
 *
 * @return NULL on error, a pointer to xmlXPathObject on sucess.
 */
xmlXPathObject *atom_get_entries(xmlDoc *document);


/** Extract information from a Atom entry (what, where, location, etc).
 *
 * @param entry Pointer to a libxml node.
 *
 * @return NULL on error, a pointer to a \ref gcal_entries structure.
 * You should clean up it using \ref gcal_destroy_entry.
 */
struct gcal_entries *atom_extract_data(xmlNode *entry);

#endif
