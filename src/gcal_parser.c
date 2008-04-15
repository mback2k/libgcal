#include "gcal_parser.h"
#include "atom_parser.h"
#include "xml_aux.h"
#include <libxml/tree.h>
#include <string.h>

struct dom_document {
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
			struct gcal_entries *data_extract, int length)
{

	int result = -1, i;
	xmlXPathObject *xpath_obj = NULL;
	xmlNodeSet *nodes;
	(void) doc;
	(void) data_extract;
	(void) length;

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

char scheme_href[] = "http://schemas.google.com/g/2005#kind";
char term_href[] = "http://schemas.google.com/g/2005#event";


int xmlentry_create(struct gcal_entries *entry, char **xml_entry, int *length)
{
	int result = -1;
	xmlDoc *doc = NULL;
	xmlNode *root, *node;
	xmlNs *ns;
	xmlChar *xml_str;

	(void)entry;
	(void)length;
	(void)node;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "entry");

	if (!doc || !root)
		goto exit;

	xmlSetProp(root, BAD_CAST "xmlns", BAD_CAST atom_href);
	ns =  xmlNewNs(root, BAD_CAST gd_href, BAD_CAST "gd");

	xmlDocSetRootElement(doc, root);

	/* TODO: mount the XML here */


	/* category element */
	node = xmlNewNode(NULL, "category");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "scheme", BAD_CAST scheme_href);
	xmlSetProp(node, BAD_CAST "term", BAD_CAST term_href);
	xmlAddChild(root, node);

	/* title element */
	node = xmlNewNode(NULL, "title");
	if (!node)
		goto cleanup;
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text");
	xmlNodeAddContent(node, entry->title);
	xmlAddChild(root, node);

	/* event status */


	/* where */


	/* when */


	xmlDocDumpMemory (doc, &xml_str, length);
	/* xmlDocDumpMemory doesn't include the last 0 in the returned size */
	++(*length);
	if (xml_str)
		if ((*xml_entry = strdup(xml_str)))
			result = 0;

cleanup:

	if (doc)
		xmlFreeDoc(doc);

exit:

	return result;

}
