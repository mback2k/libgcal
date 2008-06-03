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
#include "atom_parser.h"
#include "xml_aux.h"
#include <string.h>

int build_doc_tree(xmlDoc **document, char *xml_data)
{
	int result = -1;
	if (!xml_data)
		goto exit;

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)
	/* Only build a tree if there isn't one */
	if (!*document) {
		*document = xmlReadMemory(xml_data, strlen(xml_data),
					  "noname.xml", NULL, 0);
		if (!*document)
			goto exit;

		result = 0;
	}

#endif

exit:
	return result;

}

void clean_doc_tree(xmlDoc **document)
{
	if (document)
		if (*document)
			xmlFreeDoc(*document);
	*document = NULL;
}

int atom_entries(xmlDoc *document)
{
	int result = -1;
	xmlXPathObject *xpath_obj = NULL;
	xmlNodeSet *node;

	if (!document)
		goto exit;

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

	xpath_obj = execute_xpath_expression(document,
				       "//openSearch:totalResults/text()",
				       NULL);
	if (!xpath_obj)
		goto exit;

	node = xpath_obj->nodesetval;
	/* The expression can only return 1 node */
	if (node->nodeNr != 1)
		goto cleanup;
	/* Node type must be 'text' */
	if (strcmp(node->nodeTab[0]->name, "text") ||
	    (node->nodeTab[0]->type != XML_TEXT_NODE))
		goto cleanup;

	result = atoi(node->nodeTab[0]->content);

cleanup:
	xmlXPathFreeObject(xpath_obj);

#endif

exit:
	return result;
}

xmlXPathObject *atom_get_entries(xmlDoc *document)
{
	xmlXPathObject *xpath_obj = NULL;

	if (!document)
		goto exit;


#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

	xpath_obj = execute_xpath_expression(document,
				       "//atom:entry",
				       NULL);


#endif

exit:
	return xpath_obj;

}
static char *extract_and_check(xmlDoc *doc, char *xpath_expression, char *attr)
{
	xmlXPathObject *xpath_obj;
	char *result = NULL;
	xmlNodeSet *node;
	xmlChar *tmp;
	xpath_obj = execute_xpath_expression(doc,
					     xpath_expression,
					     NULL);

	if (!xpath_obj) {
		fprintf(stderr, "extract_and_check: failed to extract data");
		goto exit;
	}

	node = xpath_obj->nodesetval;
	/* Empty fields are set to a empty string */
	if (!node) {
		result = strdup("");
		goto cleanup;
	} else if (node->nodeNr != 1) {
		result = strdup("");
		goto cleanup;
	}

	if ((node->nodeTab[0]->type != XML_TEXT_NODE) &&
	    (node->nodeTab[0]->type != XML_ELEMENT_NODE))
		goto cleanup;

	if (node->nodeTab[0]->type == XML_TEXT_NODE) {
		if (node->nodeTab[0]->content)
			result = strdup(node->nodeTab[0]->content);
	} else if ((node->nodeTab[0]->type == XML_ELEMENT_NODE) &&
		   (attr != NULL)) {
		tmp = xmlGetProp(node->nodeTab[0], attr);
		if (!tmp)
			goto cleanup;
		result = strdup(tmp);
		xmlFree(tmp);
	}

cleanup:
	xmlXPathFreeObject(xpath_obj);
exit:
	return result;
}

int atom_extract_data(xmlNode *entry, struct gcal_entries *ptr_entry)
{
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *copy = NULL;

	if (!entry || !ptr_entry)
		goto exit;

	/* Creates a doc from this element node: yeah, nasty, I should
	 * think of a better way later...
	 */
	doc = xmlNewDoc("1.0");
	if (!doc)
		goto exit;

	copy = xmlCopyNode(entry, 1);
	if (!copy)
		goto cleanup;

	xmlDocSetRootElement(doc, copy);

	/* Gets the 'what' calendar field */
	ptr_entry->title = extract_and_check(doc,
					     "//atom:entry/atom:title/text()",
					     NULL);
	if (!ptr_entry->title)
		goto cleanup;

	/* Gets the 'id' calendar field */
	ptr_entry->id = extract_and_check(doc,
					  "//atom:entry/atom:id/text()",
					  NULL);
	if (!ptr_entry->id)
		goto cleanup;

	/* Gets the 'edit url' calendar field
	 * FIXME: I dont known how to extract attribute value from
	 * XML_ATTRIBUTE_NODE. If I discover later how to do it, it
	 * should work with the XPath expression:
	 * '//atom:entry/atom:link[@rel='edit']/@href'
	 */
	ptr_entry->edit_uri = extract_and_check(doc, "//atom:entry/"
						"atom:link[@rel='edit']",
						"href");
	if (!ptr_entry->edit_uri)
		goto cleanup;

	/* Gets the 'content' calendar field */
	ptr_entry->content = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:content/text()",
					       NULL);

	/* Gets the 'recurrent' calendar field */
	ptr_entry->dt_recurrent = extract_and_check(doc,
						    "//atom:entry/"
						    "gd:recurrence/text()",
						    NULL);

	/* Gets the when 'start' calendar field */
	ptr_entry->dt_start = extract_and_check(doc,
						"//atom:entry/gd:when",
						"startTime");
	if (!ptr_entry->dt_start)
		goto cleanup;

	/* Gets the when 'end' calendar field */
	ptr_entry->dt_end = extract_and_check(doc,
					      "//atom:entry/gd:when",
					      "endTime");
	if (!ptr_entry->dt_end)
		goto cleanup;


	/* Gets the 'where' calendar field */
	ptr_entry->where = extract_and_check(doc,
					     "//atom:entry/"
					     "gd:where",
					     "valueString");

	/* Gets the 'status' calendar field */
	ptr_entry->status = extract_and_check(doc,
					      "//atom:entry/gd:eventStatus",
					      "value");
	if (!ptr_entry->status)
		goto cleanup;


	/* Gets the 'updated' calendar field */
	ptr_entry->updated = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:updated/text()",
					       NULL);
	if (!ptr_entry->id)
		goto cleanup;

	result = 0;

cleanup:

	/* Dumps the doc to stdout */
	/* xmlSaveFormatFileEnc("-", doc, "UTF-8", 1); */
	xmlFreeDoc(doc);

exit:
	return result;
}

int atom_extract_contact(xmlNode *entry, struct gcal_contact *ptr_entry)
{
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *copy = NULL;

	if (!entry || !ptr_entry)
		goto exit;

	/* XXX: this function is pretty much a copy of 'atom_extract_data'
	 * some code could be shared if I provided a common type between
	 * contact X calendar.
	 */

	/* Creates a doc from this element node: yeah, nasty, I should
	 * think of a better way later...
	 */
	doc = xmlNewDoc("1.0");
	if (!doc)
		goto exit;

	copy = xmlCopyNode(entry, 1);
	if (!copy)
		goto cleanup;

	xmlDocSetRootElement(doc, copy);

	/* Gets the 'id' contact field */
	ptr_entry->id = extract_and_check(doc,
					  "//atom:entry/atom:id/text()",
					  NULL);
	if (!ptr_entry->id)
		goto cleanup;

	/* Gets the 'updated' contact field */
	ptr_entry->updated = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:updated/text()",
					       NULL);


	/* Gets the 'who' contact field */
	ptr_entry->title = extract_and_check(doc,
					     "//atom:entry/atom:title/text()",
					     NULL);
	if (!ptr_entry->title)
		goto cleanup;


	/* Gets the 'edit url' contact field */
	ptr_entry->edit_uri = extract_and_check(doc, "//atom:entry/"
						"atom:link[@rel='edit']",
						"href");
	if (!ptr_entry->edit_uri)
		goto cleanup;

	/* Gets the email contact field */
	ptr_entry->email = extract_and_check(doc, "//atom:entry/"
						"gd:email",
						"address");
	if (!ptr_entry->email)
		goto cleanup;

	/* Here begins extra fields */

	/* Gets the 'content' contact field */
	ptr_entry->content = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:content/text()",
					       NULL);

	/* TODO: implement remaining extra fields */
	ptr_entry->org_name = NULL;
	ptr_entry->org_title = NULL;
	ptr_entry->im = NULL;
	ptr_entry->phone_number = NULL;
	ptr_entry->post_address = NULL;


	result = 0;

cleanup:

	/* Dumps the doc to stdout */
	/* xmlSaveFormatFileEnc("-", doc, "UTF-8", 1); */
	xmlFreeDoc(doc);

exit:
	return result;
}
