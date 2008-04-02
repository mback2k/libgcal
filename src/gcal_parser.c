#include "gcal_parser.h"
#include "atom_parser.h"
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

	int result = -1;
	(void) doc;
	(void) data_extract;
	(void) length;

	/* TODO: get the entry node list */


	/* TODO: extract the fields */

	return result;
}
