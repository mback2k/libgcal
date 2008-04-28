/*
Copyright (c) 2008 Instituto Nokia de Tecnologia
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the INdT nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @file   atom_parser.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
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
 * \todo check which fields are optional and which are mandatory
 *
 * @param entry Pointer to a libxml node.
 *
 * @param ptr_entry Pointer to a libgcal entry (see \ref gcal_entries).
 *
 * @return 0 on sucess, -1 otherwise.
 */
int atom_extract_data(xmlNode *entry, struct gcal_entries *ptr_entry);

#endif
