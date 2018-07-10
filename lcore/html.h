#ifndef __HTML_H
#define __HTML_H

#include "lua.hpp"
#include "html/ParserDom.h"
#include "html/Node.h"
#include "html/utils.h"

typedef struct {
	htmlcxx::HTML::ParserDom* m_pParser;
	tree<htmlcxx::HTML::Node>* m_pDom;
} html_t;

typedef struct {
	tree<htmlcxx::HTML::Node>::iterator m_Iter;
	htmlcxx::HTML::Node* m_pNode;
} node_t;

void luaf_makenode(lua_State* L,tree<htmlcxx::HTML::Node>::iterator& it);
void luaf_makehtml(lua_State* L,std::string& code);

void libhtml_open(lua_State* L);

#endif