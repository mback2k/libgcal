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
#ifndef __GCAL_XML_AUX__
#define __GCAL_XML_AUX__

/**
 * @file   xml_aux.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Fri Mar 28 14:31:21 2008
 *
 * @brief  Auxiliary code to parse XML using XPath, iIt depends on libxml.
 *
 * I started with 'xpath1.c' libxml example written by Aleksey Sanin
 * (which uses MIT license).
 * In the end my code turn out to be rather different from him, but
 * I decided to keep the same function names.
 *
 */

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlwriter.h>

/** Atom URL/URI (ps: Its shared with \ref gcal_parser) */
static const char atom_href[] = "http://www.w3.org/2005/Atom";
static const char atom_ns[] = "atom";

/** Google data URL/URI */
static const char gd_href[] = "http://schemas.google.com/g/2005";
static const char gd_ns[] = "gd";

/** Google group membership URL/URI */
static const char gContact_href[] = "http://schemas.google.com/contact/2008";
static const char gContact_ns[] = "gContact";

/** Google calendar URL/URI */
static const char gcal_href[] = "http://schemas.google.com/gCal/2005";
static const char gcal_ns[] = "gCal";

/** Opensearch URL/URI */
static const char open_search_href[] = "http://a9.com/-/spec/opensearch/1.1/";
static const char open_search_ns[] = "openSearch";



/** Call this function to register a namespace within a xmlXPathContext.
 *
 *
 * @param xpathCtx A pointer to a libxml:xmlXPathContext.
 *
 * @param name_space Namespace name (e.g. 'gd' for google data). With NULL
 * it will register gcalendar required namespaces (gd, atom, openSearch).
 *
 * @param href A URL/URI reference to the namespace. Pass NULL to register
 * gcalendar namespaces.
 *
 * @return 0 on success, -1 otherwise.
 */
int register_namespaces(xmlXPathContext *xpathCtx, const xmlChar *name_space,
			const xmlChar* href);


/** Executes a XPath expression within a XML tree document.
 *
 *
 * @param doc A libxml document pointer.
 *
 * @param xpathExpr A pointer to a string with the xpath expression
 * (e.g. '//openSearch:totalResults/text()')
 *
 * @param xpathCtx Pointer to a xmlXPathContext (which you can configure its
 * namespaces using \ref register_namespaces). If you wish to use the default
 * gcalendar namespaces, pass NULL.
 *
 * @return A pointer to a xmlXPathObject with the result of XPath expression
 * (you must cleanup its memory using 'xmlXPathFreeObject').
 */
xmlXPathObject* execute_xpath_expression(xmlDoc *doc,
					 const xmlChar* xpathExpr,
					 xmlXPathContext *xpathCtx);

/** Allocates resources to create a XML document.
 *
 *
 * @param writer Pointer to pointer to a libxml TextWriter.
 *
 * @param buffer Pointer to pointer to a libxml buffer.
 *
 * @return 0 on sucess, -1 on error.
 */
int xmlentry_init_resources(xmlTextWriter **writer, xmlBuffer **buffer);


/** Destroys resources required to create a XML document.
 *
 *
 * @param writer Pointer to pointer to a libxml TextWriter.
 *
 * @param buffer Pointer to pointer to a libxml buffer.
 */
void xmlentry_destroy_resources(xmlTextWriter **writer, xmlBuffer **buffer);

#endif
