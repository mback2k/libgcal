#ifndef __GCAL_XML_AUX__
#define __GCAL_XML_AUX__

/**
 * @file   xml_aux.h
 * @author teste
 * @date   Fri Mar 28 14:31:21 2008
 *
 * @brief  Auxiliary code to parse XML using XPath.
 *
 * It depends on libxml, and I based it on 'xpath1.c'
 * libxml example written by Aleksey Sanin (which I think
 * uses MIT license).
 *
 */

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>



int register_namespaces(xmlXPathContext *xpathCtx, const xmlChar *name_space,
			const xmlChar* href);


xmlXPathObject* execute_xpath_expression(xmlDoc *doc,
					 const xmlChar* xpathExpr,
					 xmlXPathContext *xpathCtx);


#endif
