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
 * @file   atom_parser.c
 * @author Adenilson Cavalcanti <adenilson.silva@indt.org.br>
 * @date   Fri Mar 28 08:07:43 2008
 *
 * @brief  This is the Atom XML feed parser, it provides functions
 * to get the number of events, extract data from those events and
 * so on.
 *
 * It depends on libxml2.
 *
 */

#include "atom_parser.h"
#include "xml_aux.h"
#include "internal_gcal.h"
#include <string.h>


void workaround_edit_url(char *inplace)
{
	char *aux, *tmp;
	const char *aux2;
	int length_user, length_original = 0;
	const char start[] = "feeds/";
	const char end[] = "/private/";

	const char target[] = "%40";
	/* It is save, since it has 7 characters and an email
	 * will have at least 9 (e.g. "a%40f.com")
	 */
	const char *replacement = "default";


	if (!strstr(inplace, target))
		return;

	length_original = strlen(inplace);

	if (!(aux = strstr(inplace, start)))
		return;
	aux += sizeof(start) - 1;

	if (!(tmp = strstr(inplace, end)))
		return;

	/* The lenght of user account substring */
	length_user = tmp - aux;

	/* Replace the stuff */
	aux2 = replacement;
	while (*aux2)
		*aux++ = *aux2++;

	while (*tmp)
		*aux++ = *tmp++;

	inplace[length_original - (length_user - strlen(replacement))] = '\0';
}

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

	if ((node = xpath_obj->nodesetval)) {
		/* The expression can only return 1 node */
		if (node->nodeNr != 1)
			goto cleanup;
	} else
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
		fprintf(stderr, "extract_and_check: failed to extract data\n");
		fprintf(stderr, "xpath_expression: ---%s---\n",
			xpath_expression);
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

static int extract_and_check_multi(xmlDoc *doc, char *xpath_expression,
				   int getContent, char *attr1, char *attr2,
				   char* attr3, char ***values, char ***types,
				   int *pref)
{
	xmlXPathObject *xpath_obj;
	xmlNodeSet *node;
	xmlChar *tmp;
	int result = -1;
	int i;

	xpath_obj = execute_xpath_expression(doc,
					     xpath_expression,
					     NULL);

	if ((!values) || (attr2 && !types) || (attr3 && !pref)) {
		fprintf(stderr, "extract_and_check_multi: null pointers received");
		goto exit;
	}

	if (!xpath_obj) {
		fprintf(stderr, "extract_and_check_multi: failed to extract data");
		goto exit;
	}

	node = xpath_obj->nodesetval;

	if (!node) {
		result = 0;
		goto cleanup;
	}
	result = node->nodeNr;

	if (result == 0) {
		goto exit;
	}

	*values = (char **)malloc(node->nodeNr * sizeof(char*));
	if (attr2)
		*types = (char **)malloc(node->nodeNr * sizeof(char*));

	for (i = 0; i < node->nodeNr; i++) {
		if (getContent)
			(*values)[i] = xmlNodeGetContent(node->nodeTab[i]);
		else if (xmlHasProp(node->nodeTab[i], attr1))
			(*values)[i] = xmlGetProp(node->nodeTab[i], attr1);
		else
			(*values)[i] = strdup(" ");

		if (attr2) {
			if (xmlHasProp(node->nodeTab[i], attr2)) {
				tmp = xmlGetProp(node->nodeTab[i], attr2);
				(*types)[i] = strdup(strchr(tmp,'#') + 1);
				xmlFree(tmp);
			}
			else
				(*types)[i] = strdup("");
		}

		if (attr3) {
			if (xmlHasProp(node->nodeTab[i], attr3)) {
				tmp = xmlGetProp(node->nodeTab[i], attr3);
				if (!strcmp(tmp,"true"))
					*pref = i;
				xmlFree(tmp);
			}
		}
	}

cleanup:
	xmlXPathFreeObject(xpath_obj);
exit:
	return result;
}

static int extract_and_check_multisub(xmlDoc *doc, char *xpath_expression,
				   int getContent, char *attr1,
				   char* attr2, struct gcal_structured_subvalues **values, char ***types,
				   int *pref)
{
	xmlXPathObject *xpath_obj;
	xmlNodeSet *node;
	xmlNode *child, *cur_node;
	xmlChar *tmp;
	struct gcal_structured_subvalues *tempval;
	int result = -1;
	int i;

	xpath_obj = execute_xpath_expression(doc,
					     xpath_expression,
					     NULL);

	if ((!values) || (attr1 && !types) || (attr2 && !pref)) {
		fprintf(stderr, "extract_and_check_multisub: null pointers received");
		goto exit;
	}

	if (!xpath_obj) {
		fprintf(stderr, "extract_and_check_multisub: failed to extract data");
		fprintf(stderr, "xpath_expression: ---%s---\n",xpath_expression);
		goto exit;
	}

	node = xpath_obj->nodesetval;

	if (!node) {
		result = 0;
		goto cleanup;
	}
	result = node->nodeNr;

	if (result == 0)
		goto exit;

	tempval = (struct gcal_structured_subvalues *)malloc(sizeof(struct gcal_structured_subvalues));
	tempval->next_field = NULL;
	(*values) = tempval;
	if (attr1)
		*types = (char **)malloc(node->nodeNr * sizeof(char*));

	for (i = 0; i < node->nodeNr; i++) {
		if (getContent) {
			cur_node = node->nodeTab[i]->children;
			for (child = cur_node; child; child = child->next) {
				if (tempval->next_field == NULL) {
					tempval->next_field = (struct gcal_structured_subvalues *)malloc(sizeof(struct gcal_structured_subvalues));
					tempval->field_typenr = i;
					tempval->field_key = strdup(child->name);
					tmp = xmlNodeGetContent(child);
					tempval->field_value = strdup(tmp);
					free(tmp);
					tempval = tempval->next_field;
					/* init next entry */
					tempval->field_typenr = 0;
					tempval->field_key = NULL;
					tempval->field_value = NULL;
					tempval->next_field = NULL;
				}
			}
		}

		if (attr1) {
			if (xmlHasProp(node->nodeTab[i], attr1)) {
				tmp = xmlGetProp(node->nodeTab[i], attr1);
				(*types)[i] = strdup(strchr(tmp,'#') + 1);
				xmlFree(tmp);
			} else
				(*types)[i] = strdup("");
		}

		if (attr2) {
			if (xmlHasProp(node->nodeTab[i], attr2)) {
				tmp = xmlGetProp(node->nodeTab[i], attr2);
				if (!strcmp(tmp,"true"))
					*pref = i;
				xmlFree(tmp);
			}
		}
	}

cleanup:
	xmlXPathFreeObject(xpath_obj);
exit:
	return result;
}

char *get_etag_attribute(xmlNode * a_node)
{
	xmlChar *uri = NULL;
	char *result = NULL;
	if (xmlHasProp(a_node, "etag")) {
		uri = xmlGetProp(a_node, "etag");
		if (uri) {
			result = strdup(uri);
			xmlFree(uri);
		}
	}

	return result;
}


int atom_extract_data(xmlNode *entry, struct gcal_event *ptr_entry)
{
	int result = -1, length = 0;
	xmlChar *xml_str = NULL;
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

	/* Google Data API 2.0 requires ETag to edit an entry */
	/* //atom:entry/@gd:etag*/
	ptr_entry->common.etag = get_etag_attribute(copy);
	if (!ptr_entry->common.etag) {
		fprintf(stderr, "failed getting ETag!!!!!!\n");
		goto cleanup;
	}

	/* Store XML raw data */
	if (ptr_entry->common.store_xml) {
		xmlDocDumpMemory(doc, &xml_str, &length);
		if (xml_str) {
			if (!(ptr_entry->common.xml = strdup(xml_str)))
				goto cleanup;
		}
		else
			goto cleanup;
	} else
		if (!(ptr_entry->common.xml = strdup("")))
			goto cleanup;

	/* Gets the 'what' calendar field */
	ptr_entry->common.title = extract_and_check(doc,
					     "//atom:entry/atom:title/text()",
					     NULL);
	if (!ptr_entry->common.title)
		goto cleanup;

	/* Gets the 'id' calendar field */
	ptr_entry->common.id = extract_and_check(doc,
					  "//atom:entry/atom:id/text()",
					  NULL);
	if (!ptr_entry->common.id)
		goto cleanup;

	/* Gets the 'edit url' calendar field
	 * FIXME: I dont known how to extract attribute value from
	 * XML_ATTRIBUTE_NODE. If I discover later how to do it, it
	 * should work with the XPath expression:
	 * '//atom:entry/atom:link[@rel='edit']/@href'
	 */
	ptr_entry->common.edit_uri = extract_and_check(doc, "//atom:entry/"
						"atom:link[@rel='edit']",
						"href");
	if (!ptr_entry->common.edit_uri)
		goto cleanup;
	/* XXX: Starting with gcalendar protocol 2.1, the edit URL is
	 * different between a just added event versus a retrieved event.
	 * This makes the same event to have 2 distinct urls and breaks
	 * the akonadi resource (because I use it as the remoteID of
	 * item).
	 * The 'alternate' link is the same but doesn't work.
	 * See further info here:
	 * http://groups.google.com/group/google-calendar-help-dataapi/browse_thread/thread/a5cb021dd6fa5d9c
	 */
	workaround_edit_url(ptr_entry->common.edit_uri);

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

	/* Detects if event was deleted/canceled and marks the flag */
	if (!(strcmp("http://schemas.google.com/g/2005#event.canceled",
		     ptr_entry->status)))
		ptr_entry->common.deleted = 1;
	else
		ptr_entry->common.deleted = 0;

	/* Gets the 'updated' calendar field */
	ptr_entry->common.updated = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:updated/text()",
					       NULL);
	if (!ptr_entry->common.updated)
		goto cleanup;

	result = 0;

cleanup:

	/* Dumps the doc to stdout */
	/* xmlSaveFormatFileEnc("-", doc, "UTF-8", 1); */
	xmlFreeDoc(doc);
	if (xml_str)
		xmlFree(xml_str);

exit:
	return result;
}

int atom_extract_contact(xmlNode *entry, struct gcal_contact *ptr_entry)
{
	int result = -1, length = 0;
	//int j, t;
	//char *atom_str = NULL;
	char *tmp;
	xmlChar *xml_str = NULL;
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

	/* Google Data API 2.0 requires ETag to edit an entry */
	/* //atom:entry/@gd:etag*/
	ptr_entry->common.etag = get_etag_attribute(copy);
	if (!ptr_entry->common.etag) {
		fprintf(stderr, "failed getting ETag!!!!!!\n");
		goto cleanup;
	}

	/* Store XML raw data */
	if (ptr_entry->common.store_xml) {
		xmlDocDumpMemory(doc, &xml_str, &length);
		if (xml_str) {
			if (!(ptr_entry->common.xml = strdup(xml_str)))
				goto cleanup;
		}
		else
			goto cleanup;
	} else
		if (!(ptr_entry->common.xml = strdup("")))
			goto cleanup;

	/* Detects if this contacts was deleted */
	tmp = extract_and_check(doc, "//atom:entry/gd:deleted", NULL);
	if (tmp) {
		free(tmp);
		ptr_entry->common.deleted = 0;
	} else
		ptr_entry->common.deleted = 1;

	/* Gets the 'id' contact field */
	ptr_entry->common.id = extract_and_check(doc,
					  "//atom:entry/atom:id/text()",
					  NULL);
	if (!ptr_entry->common.id)
		goto cleanup;

	/* Gets the 'updated' contact field */
	ptr_entry->common.updated = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:updated/text()",
					       NULL);


	ptr_entry->structured_name_nr = extract_and_check_multisub(doc,
						    "//atom:entry/"
						    "gd:name",
						    1,
						    NULL,
						    NULL,
						    &ptr_entry->structured_name,
						    NULL,
						    NULL);

	/* The 'who' contact field changed in GData-Version: 3.0 API, see:
	 * http://code.google.com/intl/en-EN/apis/contacts/docs/3.0/
	 * migration_guide.html#Protocol
	 */
	ptr_entry->common.title = extract_and_check(doc, "//atom:entry"
						    "/gd:name/gd:fullName/text()",
						    NULL);


	if (!ptr_entry->common.title && !ptr_entry->structured_name_nr)
		goto cleanup;

	/* Gets the 'edit url' contact field */
	ptr_entry->common.edit_uri = extract_and_check(doc, "//atom:entry/"
						"atom:link[@rel='edit']",
						"href");
	if (!ptr_entry->common.edit_uri)
		goto cleanup;

	/* Gets email addressess */
	ptr_entry->emails_nr = extract_and_check_multi(doc,
						    "//atom:entry/"
						    "gd:email",
						    0,
						    "address",
						    "rel",
						    "primary",
						    &ptr_entry->emails_field,
						    &ptr_entry->emails_type,
						    &ptr_entry->pref_email);

	/* TODO Commented to allow contacts without an email address
	if (!ptr_entry->email)
		goto cleanup; */

	/* Here begins extra fields */

	/* Gets the 'content' contact field */
	ptr_entry->content = extract_and_check(doc,
					       "//atom:entry/"
					       "atom:content/text()",
					       NULL);

	/* Gets contact nickname */
	ptr_entry->nickname = extract_and_check(doc,
						"//atom:entry/"
						"gContact:nickname/text()",
						NULL);

	/* Gets the 'homepage' contact field */
	ptr_entry->homepage = extract_and_check(doc, "//atom:entry/"
						"gContact:website[@rel='home-page']",
						"href");

	/* Gets the 'blog' contact field */
	ptr_entry->blog = extract_and_check(doc, "//atom:entry/"
						"gContact:website[@rel='blog']",
						"href");

	/* Gets the organization contact field */
	ptr_entry->org_name = extract_and_check(doc,
						"//atom:entry/"
						"gd:organization/"
						"gd:orgName/text()",
						NULL);

	/* Gets the org. title contact field */
	ptr_entry->org_title = extract_and_check(doc,
						"//atom:entry/"
						"gd:organization/"
						"gd:orgTitle/text()",
						NULL);

	/* Gets the occupation/profession contact field */
	ptr_entry->occupation = extract_and_check(doc,
						"//atom:entry/"
						"gContact:occupation/text()",
						NULL);

	/* Gets contact phone numbers */
	ptr_entry->phone_numbers_nr = extract_and_check_multi(doc,
						    "//atom:entry/"
						    "gd:phoneNumber",
						    1,
						    NULL,
						    "rel",
						    NULL,
						    &ptr_entry->phone_numbers_field,
						    &ptr_entry->phone_numbers_type,
						    NULL);
	
	/* The 'postalAddress' contact field changed in GData-Version: 3.0 API, see:
	 * http://code.google.com/intl/en-EN/apis/contacts/docs/3.0/
	 * migration_guide.html#Protocol
	 */
	ptr_entry->post_address = extract_and_check(doc,
				"//atom:entry/"
				"gd:structuredPostalAddress/"
				"gd:formattedAddress/text()",
				NULL);

	/* Gets contact structured postal addressees (Google API 3.0) */
	ptr_entry->structured_address_nr = extract_and_check_multisub(doc,
						    "//atom:entry/"
						    "gd:structuredPostalAddress",
						    1,
						    "rel",
						    NULL,
						    &ptr_entry->structured_address,
						    &ptr_entry->structured_address_type,
						    NULL);

	/* Gets contact group membership info */
	ptr_entry->groupMembership_nr = extract_and_check_multi(doc,
						    "//atom:entry/"
						    "gContact:groupMembershipInfo[@deleted='false']",
						    0,
						    "href",
						    NULL,
						    NULL,
						    &ptr_entry->groupMembership,
						    NULL,
						    NULL);

	/* Gets contact birthday */
	ptr_entry->birthday = extract_and_check(doc,
					            "//atom:entry/"
						    "gContact:birthday",
						    "when");

	/* Gets contact photo edit url and test for etag */
	ptr_entry->photo = extract_and_check(doc, "//atom:entry/"
					     "atom:link[@type='image/*']",
					     "href");
	tmp = extract_and_check(doc, "//atom:entry/"
				"atom:link[@type='image/*']",
				"etag");
	if (tmp) {
		ptr_entry->photo_length = 1;
		free(tmp);
	}


	/* TODO: implement remaining extra fields */
	ptr_entry->im = NULL;


	result = 0;

cleanup:

	/* Dumps the doc to stdout */
	/* xmlSaveFormatFileEnc("-", doc, "UTF-8", 1); */
	xmlFreeDoc(doc);
	if (xml_str)
		xmlFree(xml_str);

exit:
	return result;
}
