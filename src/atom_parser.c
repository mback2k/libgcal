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

	(void)document;
	return NULL;

}


struct gcal_entries *atom_extract_data(xmlNode *entry)
{
	(void)entry;
	return NULL;
}
