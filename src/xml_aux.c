#include "xml_aux.h"
#include <assert.h>

const char atom_href[] = "http://www.w3.org/2005/Atom";
const char atom_ns[] = "atom";

const char gd_href[] = "http://schemas.google.com/g/2005";
const char gd_ns[] = "gd";

const char open_search_href[] = "http://a9.com/-/spec/opensearchrss/1.0/";
const char open_search_ns[] = "openSearch";

int register_namespaces(xmlXPathContext *xpathCtx, const xmlChar *name_space,
			const xmlChar* href)
{
	int result = -1;
	assert(xpathCtx);
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
	(void) xpathExpr;
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
