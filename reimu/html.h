#ifndef __HTML_H
#define __HTML_H

#include "lua.hpp"
#include "html/ParserDom.h"
#include "html/utils.h"
#include "html/Node.h"

using namespace htmlcxx;

typedef struct {
	HTML::ParserDom* m_pParserDom;
	tree<HTML::Node>* m_pDom;
} html_t;

typedef struct {
	HTML::Node* m_pNode;
	tree<HTML::Node>::iterator* m_pIter;
} node_t;

int luaopen_html(lua_State* L);

#endif