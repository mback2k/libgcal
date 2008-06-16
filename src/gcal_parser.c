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
 * @file   gcal_parser.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar 31 11:17:02 2008
 *
 * @brief  A thin layer over \ref atom_parser.h, so I can plug another
 * XML parser to libgcal if required.
 * It creates a DOM document from libgcal atom stream and provides functions
 * wrappers to extract data.
 */

#include "gcal_parser.h"
#include "atom_parser.h"
#include "xml_aux.h"
#include <libxml/tree.h>
#include <string.h>

char scheme_href[] = "http://schemas.google.com/g/2005#kind";
char term_href_cal[] = "http://schemas.google.com/g/2005#event";
char term_href_cont[] = "http://schemas.google.com/contact/2008#contact";
/** A thin wrapper around libxml document structure
 *
 */
struct dom_document {
	/** libxml DOM document structure pointer */
	xmlDoc *document;
};

/* REMARK: this function is recursive, I'm not completely sure if this
 * is a good idea (i.e. for small devices).
 */
static char *get(xmlNode *a_node)
{
	xmlNode *cur_node = NULL;
	char *result = NULL;
	xmlChar *uri = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (xmlHasProp(cur_node, "HREF")) {
			uri = xmlGetProp(cur_node, "HREF");
			if (uri) {
				result = strdup(uri);
				xmlFree(uri);
				goto exit;
			}

		}

		result = get(cur_node->children);
		if (result)
			goto exit;
	}

exit:
	return result;

}

int get_the_url(char *data, int length, char **url)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	int result = -1;

	*url = NULL;
	doc = xmlReadMemory(data, length, "noname.xml", NULL, 0);
	if (!doc)
		goto exit;

	root_element = xmlDocGetRootElement(doc);
	*url = get(root_element);

	xmlFreeDoc(doc);
	xmlCleanupParser();
	result = 0;
exit:
	return result;
}

dom_document *build_dom_document(char *xml_data)
{
	dom_document *ptr = NULL;
	if (!xml_data)
		goto exit;

	if (build_doc_tree(&ptr, xml_data)) {
		fprintf(stderr, "build_dom_document: failed doc parse");
		goto cleanup;
	}

	goto exit;

cleanup:
	if (ptr)
		free(ptr);

exit:
	return ptr;
}


void clean_dom_document(dom_document *doc)
{
	if (doc)
		clean_doc_tree(&doc);

}

int get_entries_number(dom_document *doc)
{
	int result = -1;
	if (!doc) {
		fprintf(stderr, "get_entries_number: null document!");
		goto exit;
	}

	result = atom_entries(doc);
exit:
	return result;
}

int extract_all_entries(dom_document *doc,
			struct gcal_event *data_extract, int length)
{

	int result = -1, i;
	xmlXPathObject *xpath_obj = NULL;
	xmlNodeSet *nodes;

	/* get the entry node list */
	xpath_obj = atom_get_entries(doc);
	if (!xpath_obj)
		goto exit;
	nodes = xpath_obj->nodesetval;
	if (!nodes)
		goto exit;

	if (length != nodes->nodeNr) {
		fprintf(stderr, "extract_all_entries: Size mismatch!");
		goto cleanup;
	}

	/* extract the fields */
	for (i = 0; i < length; ++i) {
		result = atom_extract_data(nodes->nodeTab[i], &data_extract[i]);
		if (result == -1)
			goto cleanup;
	}

	result = 0;

cleanup:
	xmlXPathFreeObject(xpath_obj);

exit:
	return result;
}

int xmlentry_create(struct gcal_event *entry, char **xml_entry, int *length)
{
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *root, *node;
	xmlNs *ns;
	xmlChar *xml_str = NULL;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "entry");

	if (!doc || !root)
		goto exit;

	xmlSetProp(root, BAD_CAST "xmlns", BAD_CAST atom_href);
	ns =  xmlNewNs(root, BAD_CAST gd_href, BAD_CAST "gd");

	xmlDocSetRootElement(doc, root);


	/* entry ID, only if the 'entry' is already existant (i.e. the user
	 * of library just got one entry result from a request from
	 * server).
	 */
	if (entry->id) {
		node = xmlNewNode(NULL, "id");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, entry->id);
		xmlAddChild(root, node);
	}

	/* category element */
	node = xmlNewNode(NULL, "category");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "scheme", BAD_CAST scheme_href);
	xmlSetProp(node, BAD_CAST "term", BAD_CAST term_href_cal);
	xmlAddChild(root, node);

	/* title element */
	node = xmlNewNode(NULL, "title");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
	xmlNodeAddContent(node, entry->title);
	xmlAddChild(root, node);

	/* content element */
	node = xmlNewNode(NULL, "content");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
	xmlNodeAddContent(node, entry->content);
	xmlAddChild(root, node);

	/* entry edit URL, only if the 'entry' is already existant.
	 */
	if (entry->edit_uri) {
		node = xmlNewNode(NULL, "link");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "edit");
		xmlSetProp(node, BAD_CAST "type",
			   BAD_CAST "application/atom+xml");
		xmlSetProp(node, BAD_CAST "href",
			   BAD_CAST entry->edit_uri);
		xmlAddChild(root, node);

	}


	/* transparency */
	node = xmlNewNode(ns, "transparency");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "value",
		   BAD_CAST "http://schemas.google.com/g/2005#event.opaque");
	xmlAddChild(root, node);

	/* event status */
	node = xmlNewNode(ns, "eventStatus");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "value",
		   BAD_CAST "http://schemas.google.com/g/2005#event.confirmed");
	xmlAddChild(root, node);


	/* where */
	if (entry->where) {
		node = xmlNewNode(ns, "where");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "valueString", BAD_CAST entry->where);
		xmlAddChild(root, node);
	}

	/* when */
	node = xmlNewNode(ns, "when");
	if (!node)
		goto cleanup;
	if (entry->dt_start)
		xmlSetProp(node, BAD_CAST "startTime",
			   BAD_CAST entry->dt_start);
	if (entry->dt_end)
		xmlSetProp(node, BAD_CAST "endTime", BAD_CAST entry->dt_end);
	xmlAddChild(root, node);


	xmlDocDumpMemory(doc, &xml_str, length);
	/* xmlDocDumpMemory doesn't include the last 0 in the returned size */
	++(*length);
	if (xml_str)
		if ((*xml_entry = strdup(xml_str)))
			result = 0;

cleanup:

	if (xml_str)
		xmlFree(xml_str);

	if (doc)
		xmlFreeDoc(doc);

exit:

	return result;

}

int extract_all_contacts(dom_document *doc,
			struct gcal_contact *data_extract, int length)
{

	/* The logic of this function is the same of 'extract_all_entries'
	 * but I can't find a way to share code without having a common
	 * type for contact/calendar and registering a callback which
	 * would accept both types as a valid parameter and parse the
	 * DOM outputing a vector of contacts/entries.
	 */
	int result = -1, i;
	xmlXPathObject *xpath_obj = NULL;
	xmlNodeSet *nodes;

	/* get the contact node list */
	xpath_obj = atom_get_entries(doc);
	if (!xpath_obj)
		goto exit;
	nodes = xpath_obj->nodesetval;
	if (!nodes)
		goto exit;

	if (length != nodes->nodeNr) {
		fprintf(stderr, "extract_all_contacts: Size mismatch!");
		goto cleanup;
	}

	/* extract the fields */
	for (i = 0; i < length; ++i) {
		result = atom_extract_contact(nodes->nodeTab[i],
					      &data_extract[i]);
		if (result == -1)
			goto cleanup;
	}

	result = 0;

cleanup:
	xmlXPathFreeObject(xpath_obj);

exit:
	return result;
}

int xmlcontact_create(struct gcal_contact *contact, char **xml_contact,
		      int *length)
{
	/* XXX: this function is pretty much a copy of 'xmlentry_create'
	 * some code could be shared if I provided a common type between
	 * contact X calendar.
	 */
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *root, *node;
	xmlNs *ns;
	xmlChar *xml_str = NULL;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "entry");

	if (!doc || !root)
		goto exit;

	xmlSetProp(root, BAD_CAST "xmlns", BAD_CAST atom_href);
	ns =  xmlNewNs(root, BAD_CAST gd_href, BAD_CAST "gd");

	xmlDocSetRootElement(doc, root);

	/* category element */
	node = xmlNewNode(NULL, "category");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "scheme", BAD_CAST scheme_href);
	xmlSetProp(node, BAD_CAST "term", BAD_CAST term_href_cont);
	xmlAddChild(root, node);

	/* entry ID, only if the 'contact' is already existant (i.e. the user
	 * of library just got one contact result from a request from
	 * server).
	 */
	if (contact->id) {
		node = xmlNewNode(NULL, "id");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, contact->id);
		xmlAddChild(root, node);
	}

	/* title element */
	node = xmlNewNode(NULL, "title");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
	xmlNodeAddContent(node, contact->title);
	xmlAddChild(root, node);

	/* entry edit URL, only if the 'entry' is already existant.
	 */
	if (contact->edit_uri) {
		node = xmlNewNode(NULL, "link");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "edit");
		xmlSetProp(node, BAD_CAST "type",
			   BAD_CAST "application/atom+xml");
		xmlSetProp(node, BAD_CAST "href",
			   BAD_CAST contact->edit_uri);
		xmlAddChild(root, node);

	}

	/* There are 3 types of e-mail: other, work, home */
	node = xmlNewNode(ns, "email");
	if (!node)
		goto cleanup;

	xmlSetProp(node, BAD_CAST "rel",
		   BAD_CAST "http://schemas.google.com/g/2005#other");
	xmlSetProp(node, BAD_CAST "address",
		   BAD_CAST contact->email);
	xmlAddChild(root, node);


	/* Here begin extra fields */
	/* content element */
	node = xmlNewNode(NULL, "content");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
	xmlNodeAddContent(node, contact->content);
	xmlAddChild(root, node);

	/* TODO: implement missing fields (org_name, org_title, im,
	 * phone_number, post_address).
	 */

	xmlDocDumpMemory(doc, &xml_str, length);
	/* xmlDocDumpMemory doesn't include the last 0 in the returned size */
	++(*length);
	if (xml_str)
		if ((*xml_contact = strdup(xml_str)))
			result = 0;
cleanup:

	if (xml_str)
		xmlFree(xml_str);

	if (doc)
		xmlFreeDoc(doc);

exit:

	return result;
}
