#include "utils.h"
#include <string.h>

int luaL_refobj(lua_State* L)
{
	int iRef = lua_rawlen(L,LUA_REGISTRYINDEX)+1;
	lua_pushinteger(L,iRef);
	lua_pushvalue(L,-2);
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);
	return iRef;
}

void luaL_unrefobj(lua_State* L,int ref)
{
	lua_pushinteger(L,ref);
	lua_pushnil(L);
	lua_settable(L,LUA_REGISTRYINDEX);
}

void luaL_pushref(lua_State* L,int ref)
{
	lua_pushinteger(L,ref);
	lua_gettable(L,LUA_REGISTRYINDEX);
}

void luaL_openreg(lua_State* L,luaL_Reg* pTable,bool bMeta)
{
	bool bHasIndex = false;
	lua_newtable(L);
	while(pTable->func)
	{
		if(bMeta && !strcmp(pTable->name,
			"__index")) bHasIndex = true;
		lua_pushstring(L,pTable->name);
		lua_pushcfunction(L,pTable->func);
		lua_settable(L,-3);
		pTable++;
	}

	if(bMeta && !bHasIndex)
	{
		lua_pushvalue(L,-1);
		lua_setfield(L,-2,"__index");
	}
}

void luaL_registermeta(lua_State* L,const char* pName,
	luaL_Reg* pTable)
{
	lua_pushstring(L,pName);
	luaL_openreg(L,pTable,true);
	lua_settable(L,LUA_REGISTRYINDEX);
}

void luaL_pushmeta(lua_State* L,const char* pName)
{
	lua_pushstring(L,pName);
	lua_gettable(L,LUA_REGISTRYINDEX);
}