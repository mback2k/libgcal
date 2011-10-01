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
	if (*url)
		result = 0;

	xmlFreeDoc(doc);

exit:
	return result;

}

static char *get_edit(xmlNode *a_node)
{
	xmlNode *cur_node = NULL;
	char *result = NULL;
	xmlChar *attr = NULL, *uri = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (xmlHasProp(cur_node, "rel")) {
			attr = xmlGetProp(cur_node, "rel");
			if (attr) {
				if (!strcmp(attr, "edit")) {
					uri = xmlGetProp(cur_node, "href");
					if (uri)
						result = strdup(uri);
					xmlFree(attr);
					xmlFree(uri);
					goto exit;
				}

				xmlFree(attr);
			}

		}

		result = get_edit(cur_node->children);
		if (result)
			goto exit;
	}

exit:
	return result;
}

int get_edit_url(char *data, int length, char **url)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	int result = -1;

	*url = NULL;
	doc = xmlReadMemory(data, length, "noname.xml", NULL, 0);
	if (!doc)
		goto exit;

	root_element = xmlDocGetRootElement(doc);
	*url = get_edit(root_element);
	if (*url)
		result = 0;

	xmlFreeDoc(doc);

exit:
	return result;
}

int get_edit_etag(char *data, int length, char **url)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	int result = -1;

	*url = NULL;
	doc = xmlReadMemory(data, length, "noname.xml", NULL, 0);
	if (!doc)
		goto exit;

	root_element = xmlDocGetRootElement(doc);
	*url = get_etag_attribute(root_element);
	if (*url)
		result = 0;

	xmlFreeDoc(doc);

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

int get_entries_number_xml(dom_document *doc)
{
	int		result = -1;
	xmlXPathObject	*xpath_obj = NULL;
	xmlNodeSet	*nodes;

	if (!doc) {
		fprintf(stderr, "get_entry_number: null document!");
	}

	xpath_obj = atom_get_entries(doc);
	if (!xpath_obj)
		goto exit;
	nodes = xpath_obj->nodesetval;
	if (!nodes)
		goto exit;

	result = nodes->nodeNr;
	xmlXPathFreeObject(xpath_obj);

exit:
	return result;
}

int get_calendar_entry(dom_document *doc, int entry_index, struct gcal_resource *res)
{
	int			result = -1;
	xmlXPathObject		*xpath_obj = NULL;
	xmlNodeSet		*nodes;

	xpath_obj = atom_get_entries(doc);
	if (!xpath_obj)
		goto exit;

	nodes = xpath_obj->nodesetval;
	if (!nodes)
		goto cleanup;

	if (entry_index > nodes->nodeNr)
		goto cleanup;

	result = atom_extract_calendar(nodes->nodeTab[entry_index], res);

cleanup:
	xmlXPathFreeObject(xpath_obj);
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
	/* Google Data API 2.0 requires ETag to edit an entry */
	if (entry->common.etag)
		xmlSetProp(root, BAD_CAST "gd:etag",
			   BAD_CAST entry->common.etag);
	ns =  xmlNewNs(root, BAD_CAST gd_href, BAD_CAST "gd");

	xmlDocSetRootElement(doc, root);


	/* entry ID, only if the 'entry' is already existant (i.e. the user
	 * of library just got one entry result from a request from
	 * server).
	 */
	if (entry->common.id) {
		node = xmlNewNode(NULL, "id");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, entry->common.id);
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
	xmlNodeAddContent(node, entry->common.title);
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
	if (entry->common.edit_uri) {
		node = xmlNewNode(NULL, "link");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "edit");
		xmlSetProp(node, BAD_CAST "type",
			   BAD_CAST "application/atom+xml");
		xmlSetProp(node, BAD_CAST "href",
			   BAD_CAST entry->common.edit_uri);
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
	if (entry->dt_start || entry->dt_end) {
		node = xmlNewNode(ns, "when");
		if (!node)
			goto cleanup;

		if (entry->dt_start)
			xmlSetProp(node, BAD_CAST "startTime",
				   BAD_CAST entry->dt_start);
		if (entry->dt_end)
			xmlSetProp(node, BAD_CAST "endTime",
				   BAD_CAST entry->dt_end);

		xmlAddChild(root, node);
	}

	/*recurrency*/
	if (entry->dt_recurrent) {
		node = xmlNewNode(ns, "recurrence");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
		xmlNodeAddContent(node, entry->dt_recurrent);
		xmlAddChild(root, node);
	}


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
		/* FIXME: don't print to terminal! */
		fprintf(stderr, "extract_all_contacts: Size mismatch!\n");
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
	int i;
	struct gcal_structured_subvalues *this_structured_entry;
	int set_structured_entry = 0;
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;
	xmlNode *node = NULL;
	xmlNode *node2 = NULL;
	xmlNode *child = NULL;
	xmlNs *ns;
	xmlNs *ns2;
	xmlChar *xml_str = NULL;
	char *temp;
	const char * rel_prefix = "http://schemas.google.com/g/2005#";

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "atom:entry");

	if (!doc || !root)
		goto exit;

	xmlSetProp(root, BAD_CAST "xmlns:atom", BAD_CAST atom_href);
	/* Google Data API 2.0 requires ETag to edit an entry */
	if (contact->common.etag)
		xmlSetProp(root, BAD_CAST "gd:etag",
			   BAD_CAST contact->common.etag);

	ns =  xmlNewNs(root, BAD_CAST gd_href, BAD_CAST "gd");

	/* Google contact group */
	ns2 =  xmlNewNs(root, BAD_CAST gContact_href, BAD_CAST "gContact");

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
	if (contact->common.id) {
		node = xmlNewNode(NULL, "id");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, contact->common.id);
		xmlAddChild(root, node);
	}

	/* Sets contact structured name (Google API 3.0) */
	if (contact->structured_name_nr) {
		set_structured_entry = 0;
		for (this_structured_entry = contact->structured_name;
		     this_structured_entry != NULL;
		     this_structured_entry = this_structured_entry->next_field) {
			if ((this_structured_entry->field_value != NULL)) {
				if( !set_structured_entry ) {
					if (!(node = xmlNewNode(ns, "name")))
						goto cleanup;
					set_structured_entry = 1;
				}

				if (!(child = xmlNewNode(ns, BAD_CAST this_structured_entry->field_key)))
					goto cleanup;
				xmlNodeAddContent(child, BAD_CAST this_structured_entry->field_value);
				xmlAddChild(node, child);
			}
		}

		if( set_structured_entry )
			xmlAddChild(root, node);
	} else if (contact->common.title && contact->common.title[0]) {
		node = xmlNewNode(NULL, "gd:name");
		if (!node)
			goto cleanup;
		node2 = xmlNewNode(NULL, "gd:fullName");
		xmlNodeAddContent(node2, contact->common.title);
		xmlAddChild(node, node2);
		xmlAddChild(root, node);
	}

	/* entry edit URL, only if the 'entry' is already existant.
	 */
	if (contact->common.edit_uri && contact->common.edit_uri[0]) {
		node = xmlNewNode(NULL, "link");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "edit");
		xmlSetProp(node, BAD_CAST "type",
			   BAD_CAST "application/atom+xml");
		xmlSetProp(node, BAD_CAST "href",
			   BAD_CAST contact->common.edit_uri);
		xmlAddChild(root, node);

	}

	/* email addresses */
	if (contact->emails_nr > 0) {
		for (i = 0; i < contact->emails_nr; i++) {
			if (!(node = xmlNewNode(ns, "email")))
				goto cleanup;
			temp = (char *)malloc((strlen(contact->emails_type[i])+strlen(rel_prefix)+1) * sizeof(char));
			strcpy(temp, rel_prefix);
			strcat(temp, contact->emails_type[i]);
			xmlSetProp(node, BAD_CAST "rel",
				  BAD_CAST temp);
			xmlSetProp(node, BAD_CAST "address",
				  BAD_CAST contact->emails_field[i]);
			if (i == contact->pref_email)
				xmlSetProp(node, BAD_CAST "primary",
					  BAD_CAST "true");
			xmlAddChild(root, node);
			free(temp);
		}
	}

	/* Here begin extra fields */
	if (contact->content && contact->content[0]) {
		node = xmlNewNode(NULL, "atom:content");
		if (!node)
			goto cleanup;
		xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
		xmlNodeAddContent(node, contact->content);
		xmlAddChild(root, node);
	}

	if (contact->nickname && contact->nickname[0]) {
		node = xmlNewNode(NULL, "gContact:nickname");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, contact->nickname);
		xmlAddChild(root, node);
	}

	if (contact->homepage && contact->homepage[0]) {
		if (!(node = xmlNewNode(NULL, "gContact:website")))
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "home-page");
		xmlSetProp(node, BAD_CAST "href", BAD_CAST contact->homepage);
		xmlAddChild(root, node);
	}

	if (contact->blog && contact->blog[0]) {
		if (!(node = xmlNewNode(NULL, "gContact:website")))
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel", BAD_CAST "blog");
		xmlSetProp(node, BAD_CAST "href", BAD_CAST contact->blog);
		xmlAddChild(root, node);
	}

	/* organization (it has 2 subelements: orgName, orgTitle) */
	if ((contact->org_name && contact->org_name[0]) || (contact->org_title && contact->org_title[0])) {
		if (!(node = xmlNewNode(ns, "organization")))
			goto cleanup;
		xmlSetProp(node, BAD_CAST "rel",
			   BAD_CAST "http://schemas.google.com/g/2005#other");

		if (contact->org_name && contact->org_name[0]) {
			if (!(child = xmlNewNode(ns, "orgName")))
				goto cleanup;
			xmlNodeAddContent(child, contact->org_name);
			xmlAddChild(node, child);
		}


		if (contact->org_title && contact->org_title[0]) {
			if (!(child = xmlNewNode(ns, "orgTitle")))
				goto cleanup;
			xmlNodeAddContent(child, contact->org_title);
			xmlAddChild(node, child);
		}

		xmlAddChild(root, node);
	}

	if (contact->occupation && contact->occupation[0]) {
		node = xmlNewNode(NULL, "gContact:occupation");
		if (!node)
			goto cleanup;
		xmlNodeAddContent(node, contact->occupation);
		xmlAddChild(root, node);
	}

	/* Get phone numbers */
	if (contact->phone_numbers_nr > 0) {
		for (i = 0; i < contact->phone_numbers_nr; i++) {
			if (!(node = xmlNewNode(ns, "phoneNumber")))
				goto cleanup;
			/* TODO: support user setting phone type */

			temp = (char *)malloc((strlen(contact->phone_numbers_type[i])+strlen(rel_prefix)+1) * sizeof(char));
			strcpy(temp, rel_prefix);
			strcat(temp, contact->phone_numbers_type[i]);
			xmlSetProp(node, BAD_CAST "rel",
				  BAD_CAST temp);

			xmlNodeAddContent(node, contact->phone_numbers_field[i]);
			xmlAddChild(root, node);
			free(temp);
		}
	}

	/* im addresses */
	if (contact->im_nr > 0) {
		for (i = 0; i < contact->im_nr; i++) {
			if (!(node = xmlNewNode(ns, "im")))
				goto cleanup;
			temp = (char *)malloc((strlen(contact->im_type[i])+strlen(rel_prefix)+1) * sizeof(char));
			strcpy(temp, rel_prefix);
			strcat(temp, contact->im_type[i]);
			xmlSetProp(node, BAD_CAST "rel",
				  BAD_CAST temp);
			temp = (char *)malloc((strlen(contact->im_protocol[i])+strlen(rel_prefix)+1) * sizeof(char));
			strcpy(temp, rel_prefix);
			strcat(temp, contact->im_protocol[i]);
			xmlSetProp(node, BAD_CAST "protocol",
				  BAD_CAST temp);
			xmlSetProp(node, BAD_CAST "address",
				  BAD_CAST contact->im_address[i]);
			if (i == contact->im_pref)
				xmlSetProp(node, BAD_CAST "primary",
					  BAD_CAST "true");
			xmlAddChild(root, node);
			free(temp);
		}
	}

	/* Sets contact structured postal addressees (Google API 3.0) */
	/* TODO: move this to another function (identation is looking bad) */
	if (contact->structured_address_nr > 0) {
		for (i = 0; i < contact->structured_address_nr; i++) {
			set_structured_entry = 0;
			for (this_structured_entry = contact->structured_address;
			     this_structured_entry != NULL;
			     this_structured_entry = this_structured_entry->next_field) {
				if (this_structured_entry->field_value &&
				    this_structured_entry->field_key &&
				    (this_structured_entry->field_typenr == i)) {
					if (!set_structured_entry) {
						if (!(node = xmlNewNode(ns, "structuredPostalAddress")))
							goto cleanup;
						// TODO: support user settting address type
						temp = (char *)malloc((strlen(contact->structured_address_type[i])+strlen(rel_prefix)+2) * sizeof(char));
						strcpy(temp, rel_prefix);
						strcat(temp, contact->structured_address_type[i]);
						xmlSetProp(node, BAD_CAST "rel", BAD_CAST temp);
						set_structured_entry = 1;
						free(temp);
					}

					if (!(child = xmlNewNode(ns, BAD_CAST this_structured_entry->field_key)))
						goto cleanup;
					xmlNodeAddContent(child, BAD_CAST this_structured_entry->field_value);
					if (i == contact->structured_address_pref)
						xmlSetProp(node, BAD_CAST "primary",
							BAD_CAST "true");
					xmlAddChild(node, child);
				}
			}

			if (set_structured_entry)
				xmlAddChild(root, node);
		}
	} else if (contact->post_address && contact->post_address[0]) {
		node = xmlNewNode(NULL, "gd:structuredPostalAddress");
		if (!node)
			goto cleanup;
		node2 = xmlNewNode(NULL, "gd:formattedAddress");
		xmlNodeAddContent(node2, contact->post_address);
		xmlAddChild(node, node2);
		xmlAddChild(root, node);
	}

	/* Google group membership info */
	if (contact->groupMembership_nr > 0) {
		for (i = 0; i < contact->groupMembership_nr; i++) {
			if (!(node = xmlNewNode(ns2, "groupMembershipInfo")))
				goto cleanup;
			xmlSetProp(node, BAD_CAST "deleted",
				  BAD_CAST "false");
			xmlSetProp(node, BAD_CAST "href",
				  BAD_CAST contact->groupMembership[i]);
			xmlAddChild(root, node);
		}
	}

	/* birthday */
	if (contact->birthday && contact->birthday[0]) {
		/*if (!(node = xmlNewNode(NULL, BAD_CAST "gContact:birthday")))
			goto cleanup;
		xmlSetProp(node, BAD_CAST "xmlns", BAD_CAST "http://schemas.google.com/contact/2008");*/
		if (!(node = xmlNewNode(NULL, "gContact:birthday")))
			goto cleanup;
		xmlSetProp(node, BAD_CAST "when", BAD_CAST contact->birthday);
		xmlAddChild(root, node);
	}

	/* TODO: implement missing fields (which ones? geo location?)
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
