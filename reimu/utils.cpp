#include "utils.h"
#include <string.h>

void luaL_openreg(lua_State* L,luaL_Reg* pReg,
	bool bMeta)
{
	bool bHasIndex = false;
	lua_newtable(L);
	while(pReg->name)
	{
		lua_pushstring(L,pReg->name);
		lua_pushcfunction(L,pReg->func);
		lua_settable(L,-3);

		pReg++;
	}
	if(bMeta)
	{
		lua_pushvalue(L,-1);
		lua_setfield(L,-2,"__index");
	}
}

void luaL_registermeta(lua_State* L,luaL_Reg* pMeta,
	const char* pName)
{
	/*luaL_openreg(L,pMeta,true);
	lua_pushstring(L,pName);
	lua_pushvalue(L,-1);
	lua_setfield(L,-3,"__name");
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_setfield(L,LUA_REGISTRYINDEX,pName);*/

	lua_pushstring(L,pName);
	luaL_openreg(L,pMeta,true);

	lua_pushvalue(L,-2);
	lua_setfield(L,-2,"__name");

	lua_settable(L,LUA_REGISTRYINDEX);
}

void luaL_pushmeta(lua_State* L,const char* pName)
{
	lua_pushstring(L,pName);
	lua_gettable(L,LUA_REGISTRYINDEX);
	//lua_getfield(L,LUA_REGISTRYINDEX,pName);
}