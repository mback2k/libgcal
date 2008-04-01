#include "atom_parser.h"
#include "xml_aux.h"
#include <string.h>

int build_doc_tree(xmlDoc **document, char *xml_data)
{
	int result = -1;

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


int atom_extract_data(xmlNode *entry, struct gcal_entries *ptr_entry)
{
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *copy = NULL;
	xmlNodeSet *node;
	xmlXPathObject *xpath_obj = NULL;

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
	xpath_obj = execute_xpath_expression(doc,
					     "//atom:entry/atom:title/text()",
					     NULL);

	if (!xpath_obj) {
		fprintf(stderr, "atom_extract_data: failed to extract data");
		goto cleanup;
	}

	node = xpath_obj->nodesetval;
	if (node->nodeNr != 1)
		goto error;

	if (strcmp(node->nodeTab[0]->name, "text") ||
	    (node->nodeTab[0]->type != XML_TEXT_NODE))
		goto error;

	ptr_entry->title = strdup(node->nodeTab[0]->content);
	if (!ptr_entry->title)
		goto error;

	/* Gets the 'id' calendar field */

	/* Gets the 'edit url' calendar field */

	/* Gets the 'content' calendar field */

	/* Gets the 'recurrent' calendar field */

	/* Gets the when 'start' calendar field */

	/* Gets the when 'end' calendar field */

	/* Gets the 'where' calendar field */

	result = 0;

error:
	xmlXPathFreeObject(xpath_obj);

cleanup:

	/* Dumps the doc to stdout */
	/* xmlSaveFormatFileEnc("-", doc, "UTF-8", 1); */
	xmlFreeDoc(doc);

exit:
	return result;
}
