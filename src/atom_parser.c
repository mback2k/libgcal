#include "atom_parser.h"
#include <string.h>


int build_doc_tree(xmlDoc **document, char *xml_data)
{
	int result = -1;

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)
	if (!*document) {
		*document = xmlReadMemory(xml_data, strlen(xml_data),
					  "noname.xml", NULL, 0);
		if (!*document)
			goto exit;
	}

	result = 0;
#endif

exit:
	return result;

}

void clean_doc_tree(xmlDoc **document)
{
	xmlFreeDoc(*document);
	*document = NULL;
}

int atom_entries(xmlDoc *document)
{
	int result = -1;
	(void)document;

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)
	/* TODO: put code here */

#endif

	return result;
}
