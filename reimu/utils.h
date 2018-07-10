#ifndef __UTILS_H
#define __UTILS_H

#include "lua.hpp"

void luaL_openreg(lua_State* L,luaL_Reg* pReg,
	bool bMeta = false);

void luaL_registermeta(lua_State* L,luaL_Reg* pMeta,
	const char* pName);
void luaL_pushmeta(lua_State* L,const char* pName);

template<typename T>
T* luaf_checkud(lua_State* L,int idx)
{
	luaL_checktype(L,idx,LUA_TUSERDATA);
	T* pUd = (T*)lua_touserdata(L,idx);
	if(!pUd) luaL_argerror(L,idx,"wrong userdata");
	return pUd;
}

#endif