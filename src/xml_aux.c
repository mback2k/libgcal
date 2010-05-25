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

#include "xml_aux.h"

int register_namespaces(xmlXPathContext *xpathCtx, const xmlChar *name_space,
			const xmlChar* href)
{
	int result = -1;
	if (!xpathCtx)
		goto exit;
	if (name_space && href) {
		/* do register namespace */
		if(xmlXPathRegisterNs(xpathCtx, name_space, href) != 0) {
			fprintf(stderr,"Error: unable to register NS with"
				"prefix=\"%s\" and href=\"%s\"\n",
				name_space, href);
			goto exit;
		}
	}
	else {
		if (register_namespaces(xpathCtx, atom_ns, atom_href) ||
		    register_namespaces(xpathCtx, gd_ns, gd_href) ||
		    register_namespaces(xpathCtx, gContact_ns, gContact_href) ||
		    register_namespaces(xpathCtx, open_search_ns,
					open_search_href) ||
		    register_namespaces(xpathCtx, gContact_ns, gContact_href))
			goto exit;
	}

	result = 0;
exit:
	return result;

}

xmlXPathObject* execute_xpath_expression(xmlDoc *doc,
					 const xmlChar* xpathExpr,
					 xmlXPathContext *xpathCtx)
{
	unsigned char ownership = 0;
	xmlXPathObject *xpath_obj = NULL;
	if (!xpathCtx) {
		ownership = 1;
		xpathCtx = xmlXPathNewContext(doc);
		if (xpathCtx == NULL) {
			fprintf(stderr,"Error: unable to create new XPath"
				"context\n");
			goto exit;
		}

		if (register_namespaces(xpathCtx, NULL, NULL))
				goto exit;

	}

	xpath_obj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if (ownership && (xpathCtx != NULL))
		xmlXPathFreeContext(xpathCtx);

exit:
	return xpath_obj;

}

int xmlentry_init_resources(xmlTextWriter **writer, xmlBuffer **buffer)
{
	int result = -1;
	*buffer = xmlBufferCreate();
	if (!buffer)
		goto exit;

	*writer = xmlNewTextWriterMemory(*buffer, 0);
	if (!*writer)
		goto exit;

	result = 0;

exit:
	return result;

}


void xmlentry_destroy_resources(xmlTextWriter **writer, xmlBuffer **buffer)
{
	if (!writer && !buffer)
		return;

	if (writer)
		if (*writer) {
			xmlFreeTextWriter(*writer);
			*writer = NULL;
		}

	if (buffer)
		if (*buffer) {
			xmlBufferFree(*buffer);
			*buffer = NULL;
		}
}
