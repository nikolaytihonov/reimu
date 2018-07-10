#include "html.h"
#include "utils.h"

static char* s_pNodeName = "node";
static char* s_pHtmlName = "html";

//Node

void luaf_makenode(lua_State* L,tree<HTML::Node>::iterator& it)
{
	node_t* pNode = (node_t*)lua_newuserdata(L,
		sizeof(node_t));
	pNode->m_pIter = new tree<HTML::Node>::iterator(it);
	pNode->m_pNode = new HTML::Node(*it);

	luaL_pushmeta(L,s_pNodeName);
	lua_setmetatable(L,-2);
}

static int node_tostring(lua_State* L)
{
	lua_pushfstring(L,"%s %p",s_pNodeName,
		luaf_checkud<node_t>(L,1));
	return 1;
}

static int node_attribute(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	luaL_checktype(L,2,LUA_TSTRING);

	std::pair<bool,std::string> attr = 
		pNode->m_pNode->attribute(lua_tostring(L,2));
	if(attr.first) lua_pushstring(L,attr.second.c_str());
	else lua_pushnil(L);
	return 1;
}

static int node_len(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	lua_pushinteger(L,pNode->m_pNode->length());
	return 1;
}

static int node_gc(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	delete pNode->m_pIter;
	delete pNode->m_pNode;
	return 0;
}

static int node_tagName(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	lua_pushstring(L,pNode->m_pNode
		->tagName().c_str());
	return 1;
}

static int node_isTag(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	lua_pushboolean(L,pNode->
		m_pNode->isTag());
	return 1;
}

static int node_isComment(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	lua_pushboolean(L,pNode->
		m_pNode->isComment());
	return 1;
}

static int node_text(lua_State* L)
{
	node_t* pNode = luaf_checkud<node_t>(L,1);
	lua_pushlstring(L,pNode->
		m_pNode->text().c_str(),
		pNode->m_pNode->length());
	return 1;
}

static luaL_Reg node_funcs[] = {
	{"__tostring",&node_tostring},
	{"__len",&node_len},
	{"__gc",&node_gc},
	{"tagName",&node_tagName},
	{"isTag",&node_isTag},
	{"isComment",&node_isComment},
	{"attribute",&node_attribute},
	{"text",&node_text},
	{NULL,NULL}
};

//HTML

void luaf_makehtml(lua_State* L,std::string& code)
{
	html_t* pHtml = (html_t*)lua_newuserdata(L,
		sizeof(html_t));

	pHtml->m_pParserDom = new HTML::ParserDom();
	pHtml->m_pDom = new tree<HTML::Node>(
		pHtml->m_pParserDom->parseTree(code));

	luaL_pushmeta(L,s_pHtmlName);
	lua_setmetatable(L,-2);
}

static int html_tostring(lua_State* L)
{
	lua_pushfstring(L,"%s %p",s_pHtmlName,
		luaf_checkud<html_t>(L,1));
	return 1;
}

static int html_len(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);
	lua_pushinteger(L,pHtml->m_pDom->size());
	return 1;
}

static int html_gc(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);
	delete pHtml->m_pParserDom;
	delete pHtml->m_pDom;
	return 0;
}

static int html_contentOf(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);
	node_t* pNode = luaf_checkud<node_t>(L,2);

	lua_pushstring(L,HTML::get_child_content(
		*pHtml->m_pDom,*pNode->m_pIter).c_str());
	return 1;
}

static int html_toTable(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);

	int i = 1;
	lua_newtable(L);
	for(tree<HTML::Node>::iterator it = pHtml->m_pDom->
		begin(); it != pHtml->m_pDom->end(); ++it)
	{
		it->parseAttributes();

		lua_pushinteger(L,i++);
		luaf_makenode(L,it);
		lua_settable(L,-3);
	}
	return 1;
}

static int html_byTagName(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);
	luaL_checktype(L,2,LUA_TSTRING);

	int i = 1;
	std::string name = std::string(lua_tostring(L,2));
	lua_newtable(L);
	for(tree<HTML::Node>::iterator it = pHtml->m_pDom->
		begin(); it != pHtml->m_pDom->end(); ++it)
	{
		it->parseAttributes();
		if(it->tagName() == name)
		{
			it->parseAttributes();

			lua_pushinteger(L,i++);
			luaf_makenode(L,it);
			lua_settable(L,-3);
		}
	}
	return 1;
}

static int html_getChildsOf(lua_State* L)
{
	html_t* pHtml = luaf_checkud<html_t>(L,1);
	node_t* pNode = luaf_checkud<node_t>(L,2);

	lua_newtable(L);
	for(unsigned int i = 0; i < pHtml->m_pDom->
		number_of_children(*pNode->m_pIter); i++)
	{
		tree<HTML::Node>::iterator it
			= pHtml->m_pDom->child(*pNode->m_pIter,i);

		it->parseAttributes();

		lua_pushinteger(L,i+1);
		luaf_makenode(L,it);
		lua_settable(L,-3);
	}
	return 1;
}

static luaL_Reg html_funcs[] = {
	{"__tostring",&html_tostring},
	{"__len",&html_len},
	{"__gc",&html_gc},
	{"contentOf",&html_contentOf},
	{"toTable",&html_toTable},
	{"byTagName",&html_byTagName},
	{"getChildsOf",&html_getChildsOf},
	{NULL,NULL}
};

//Main

static int tohtml(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);

	luaf_makehtml(L,std::string(
		lua_tostring(L,1)));
	return 1;
}

int luaopen_html(lua_State* L)
{
	luaL_registermeta(L,html_funcs,s_pHtmlName);
	luaL_registermeta(L,node_funcs,s_pNodeName);

	lua_register(L,"tohtml",tohtml);
	return 0;
}