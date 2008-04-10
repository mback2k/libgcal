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
