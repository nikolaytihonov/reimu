#include "lcurl.h"
#include "utils.h"
#include "lauxlib.h"
#include "platform.h"

static char* s_pCurlName = "curl";

CURL* luaf_makecurl(lua_State* L)
{
	lcurl_t* pCurl = (lcurl_t*)lua_newuserdata(L,
		sizeof(lcurl_t));
	if(!(pCurl->m_pCurl = curl_easy_init()))
	{
		luaL_error(L,"curl_easy_init failed!");
		return NULL;
	}

	luaL_pushmeta(L,s_pCurlName);
	lua_setmetatable(L,-2);

	return pCurl->m_pCurl;
}

static int curl_tostring(lua_State* L)
{
	lua_pushfstring(L,"%s %p",s_pCurlName,
		luaf_checkud<lcurl_t>(L,1));
	return 1;
}

static int curl_gc(lua_State* L)
{
	lcurl_t* pCurl = luaf_checkud<lcurl_t>(L,1);
	if(pCurl->m_pCurl)
	{
		curl_easy_cleanup(pCurl->m_pCurl);
		pCurl->m_pCurl = NULL;
	}
	return 0;
}

static int curl_setOpt(lua_State* L)
{
	luaL_checktype(L,2,LUA_TNUMBER);
	lcurl_t* pCurl = luaf_checkud<lcurl_t>(L,1);
	union {
		unsigned long ulData;
		const char* pData;
	};

	if(lua_type(L,3) == LUA_TNUMBER) ulData = (unsigned long)lua_tointeger(L,3);
	else if(lua_type(L,3) == LUA_TSTRING) pData = lua_tostring(L,3);
	else luaL_argerror(L,3,"supported only number and string");

	lua_pushinteger(L,curl_easy_setopt(pCurl->m_pCurl,
		(CURLoption)lua_tointeger(L,2),pData));
	return 1;
}

static int curl_getInfo(lua_State* L)
{
	luaL_checktype(L,2,LUA_TNUMBER);
	lcurl_t* pCurl = luaf_checkud<lcurl_t>(L,1);
	long lData;

	curl_easy_getinfo(pCurl->m_pCurl,(CURLINFO)lua_tointeger(L,2),&lData);
	lua_pushinteger(L,lData);
	return 1;
}

size_t stream_writer(void* pData,size_t len,size_t memb,void* pArg)
{
	luaL_Stream* file = (luaL_Stream*)pArg;
	if(!file->f) return 0;
	return fwrite(pData,len,memb,file->f);
	return len*memb;
}

static int curl_perform(lua_State* L)
{
	lcurl_t* pCurl = luaf_checkud<lcurl_t>(L,1);

	CMemPool mem;

	curl_easy_setopt(pCurl->m_pCurl,
		CURLOPT_WRITEFUNCTION,CMemPool::Writer);
	curl_easy_setopt(pCurl->m_pCurl,
		CURLOPT_WRITEDATA,&mem);

	CURLcode res = curl_easy_perform(pCurl->m_pCurl);
	if(mem.m_pMem)
		lua_pushlstring(L,(const char*)mem.m_pMem,mem.m_uLen);
	else lua_pushnil(L);

	lua_pushinteger(L,res);
	return 2;
}

static int curl_performFile(lua_State* L)
{
	lcurl_t* pCurl = luaf_checkud<lcurl_t>(L,1);
	luaL_Stream* file = (luaL_Stream*)
		luaL_checkudata(L,2,LUA_FILEHANDLE);

	curl_easy_setopt(pCurl->m_pCurl,
		CURLOPT_WRITEFUNCTION,stream_writer);
	curl_easy_setopt(pCurl->m_pCurl,
		CURLOPT_WRITEDATA,file);

	lua_pushinteger(L,curl_easy_perform(pCurl->m_pCurl));
	return 1;
}

static luaL_Reg curl_funcs[] = {
	{"__tostring",&curl_tostring},
	{"__gc",&curl_gc},
	{"close",&curl_gc},
	{"setOpt",&curl_setOpt},
	{"getInfo",&curl_getInfo},
	{"perform",&curl_perform},
	{"performFile",&curl_performFile},
	{NULL,NULL}
};

static int curl_open(lua_State* L)
{
	luaf_makecurl(L);
	return 1;
}

#define DECLARE_ECONST(l,name)				\
	lua_pushinteger(l,name);				\
	lua_setglobal(l,#name)

int luaopen_curl(lua_State* L)
{
	luaL_registermeta(L,curl_funcs,s_pCurlName);
	lua_register(L,"curl_open",curl_open);

	DECLARE_ECONST(L,CURLOPT_URL);
	DECLARE_ECONST(L,CURLOPT_PORT);
	DECLARE_ECONST(L,CURLOPT_POST);
	DECLARE_ECONST(L,CURLOPT_PROXY);
	DECLARE_ECONST(L,CURLOPT_VERBOSE);
	DECLARE_ECONST(L,CURLOPT_TIMEOUT);
	DECLARE_ECONST(L,CURLOPT_USERAGENT);
	DECLARE_ECONST(L,CURLOPT_POSTFIELDS);
	DECLARE_ECONST(L,CURLOPT_AUTOREFERER);
	DECLARE_ECONST(L,CURLOPT_REFERER);
	DECLARE_ECONST(L,CURLOPT_COOKIE);
	DECLARE_ECONST(L,CURLOPT_COOKIEFILE);
	DECLARE_ECONST(L,CURLOPT_COOKIEJAR);
	DECLARE_ECONST(L,CURLOPT_COOKIELIST);
	DECLARE_ECONST(L,CURLOPT_FOLLOWLOCATION);
	DECLARE_ECONST(L,CURLOPT_CONNECTTIMEOUT);

	DECLARE_ECONST(L,CURLINFO_RESPONSE_CODE);
	DECLARE_ECONST(L,CURLINFO_HTTP_CODE);
	return 0;
}