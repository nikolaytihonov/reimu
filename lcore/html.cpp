#include "html.h"
#include "utils.h"

inline tree<htmlcxx::HTML::Node>* luaf_tohtml(lua_State* L,int idx);

//HTML Node

void luaf_makenode(lua_State* L,tree<htmlcxx::HTML::Node>::iterator& it)
{
	/*htmlcxx::HTML::Node* pNode = (htmlcxx::HTML::Node*)
		lua_newuserdata(L,sizeof(htmlcxx::HTML::Node));*/
	node_t* pNode = (node_t*)lua_newuserdata(L,sizeof(node_t));
	pNode->m_Iter = it;
	//pNode->m_Node = htmlcxx::HTML::Node(*it);
	pNode->m_pNode = new htmlcxx::HTML::Node(*it);
	

	luaL_pushmeta(L,"node");
	lua_setmetatable(L,-2);
}

inline node_t* luaf_tonode(lua_State* L,int idx)
{
	return (node_t*)lua_touserdata(L,idx);
}

static int node_tostring(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	lua_pushlstring(L,pNode->m_pNode->text().c_str(),
		pNode->m_pNode->length());
	return 1;
}

static int node_index(lua_State* L)
{
	luaL_checktype(L,2,LUA_TSTRING);
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	std::pair<bool,std::string> tag = 
		pNode->m_pNode->attribute(std::string(lua_tostring(L,2)));
	if(tag.first)
		lua_pushstring(L,tag.second.c_str());
	else lua_pushnil(L);
	return 1;
}

static int node_gc(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	delete pNode->m_pNode;
	return 0;
}

static int node_isTag(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	lua_pushboolean(L,pNode->m_pNode->isTag());
	return 1;
}

static int node_isComment(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	lua_pushboolean(L,pNode->m_pNode->isComment());
	return 1;
}

static int node_tagName(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	lua_pushstring(L,pNode->m_pNode->tagName().c_str());
	return 1;
}

static int node_parseAttributes(lua_State* L)
{
	node_t* pNode = luaf_tonode(L,1);
	if(!pNode) return 0;

	pNode->m_pNode->parseAttributes();
	return 0;
}

static int node_getContent(lua_State* L)
{
	if(lua_gettop(L)<2) luaL_error(L,
		"node_getContent must receive self and dom!");
	node_t* pNode = luaf_tonode(L,1);
	tree<htmlcxx::HTML::Node>* pHTML = luaf_tohtml(L,2);
	if(!pNode || !pHTML) return 0;

	std::string res;
	for(unsigned int i = 0; i < pHTML->number_of_children(pNode->m_Iter); i++)
	{
		tree<htmlcxx::HTML::Node>::iterator it = pHTML->child(
			pNode->m_Iter,i);
		if(!it->isTag() && !it->isComment()) res += it->text();
	}

	lua_pushstring(L,res.c_str());
	return 1;
}

static luaL_Reg meta_node[] = {
	{"__tostring",&node_tostring},
	{"__index",&node_index},
	{"__gc",&node_gc},
	{"isTag",&node_isTag},
	{"isComment",&node_isComment},
	{"tagName",&node_tagName},
	{"parseAttributes",&node_parseAttributes},
	{"getContent",&node_getContent},
	{NULL,NULL}
};

//HTML DOM

void luaf_makehtml(lua_State* L,std::string& code)
{
	html_t* pHTML = (html_t*)lua_newuserdata(L,
		sizeof(html_t));

	//tree<htmlcxx::HTML::Node> dom = parser.parseTree(code);
	//memcpy(pDom,&dom,sizeof(tree<htmlcxx::HTML::Node>));
	pHTML->m_pParser = new htmlcxx::HTML::ParserDom;
	pHTML->m_pDom = new tree<htmlcxx::HTML::Node>(
		pHTML->m_pParser->parseTree(code));

	luaL_pushmeta(L,"html");
	lua_setmetatable(L,-2);
}

inline tree<htmlcxx::HTML::Node>* luaf_tohtml(lua_State* L,int idx)
{
	html_t* pHTML
		= (html_t*)lua_touserdata(L,idx);
	if(!pHTML) luaL_argerror(L,idx,"expected html, got shit >:");
	else return pHTML->m_pDom;
	//return *(tree<htmlcxx::HTML::Node>**)lua_touserdata(L,idx);
	return NULL;
}

static int html_tostring(lua_State* L)
{
	lua_pushfstring(L,"html %p",lua_touserdata(L,1));
	return 1;
}

static int html_gc(lua_State* L)
{
	/*tree<htmlcxx::HTML::Node>* pHTML = luaf_tohtml(L,1);
	if(!pHTML) return 0;

	pHTML->clear();*/

	html_t* pHTML
		= (html_t*)lua_touserdata(L,1);
	pHTML->m_pDom->clear();

	delete pHTML->m_pDom;
	delete pHTML->m_pParser;

	return 0;
}

static int html_toTable(lua_State* L)
{
	tree<htmlcxx::HTML::Node>* pHTML = luaf_tohtml(L,1);
	if(!pHTML) return 0;

	int i = 0;
	lua_newtable(L);
	for(tree<htmlcxx::HTML::Node>::iterator it
		= pHTML->begin(); it != pHTML->end(); ++it)
	{
		lua_pushinteger(L,++i);
		luaf_makenode(L,it);
		lua_settable(L,-3);
	}
	return 1;
}

static int html_getByName(lua_State* L)
{
	tree<htmlcxx::HTML::Node>* pHTML = luaf_tohtml(L,1);
	if(!pHTML) return 0;

	int i = 0;
	lua_newtable(L);
	for(tree<htmlcxx::HTML::Node>::iterator it
		= pHTML->begin(); it != pHTML->end(); ++it)
	{
		if(!it->isTag()) continue;
		it->parseAttributes();

		lua_pushinteger(L,++i);
		luaf_makenode(L,it);
		lua_settable(L,-3);
	}
	return 1;
}

static luaL_Reg meta_html[] = {
	{"__tostring",&html_tostring},
	{"__gc",&html_gc},
	{"toTable",&html_toTable},
	{"getByName",&html_getByName},
	{NULL,NULL}
};

static int ltohtml(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	luaf_makehtml(L,std::string(
		lua_tostring(L,1)));
	return 1;
}

void libhtml_open(lua_State* L)
{
	luaL_registermeta(L,"node",meta_node);
	luaL_registermeta(L,"html",meta_html);

	lua_register(L,"tohtml",ltohtml);
}