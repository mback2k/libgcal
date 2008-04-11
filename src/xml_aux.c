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
		    register_namespaces(xpathCtx, open_search_ns,
					open_search_href))
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
