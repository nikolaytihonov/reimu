#ifndef __UTILS_H
#define __UTILS_H

#include "lua.hpp"

int luaL_refobj(lua_State* L);
void luaL_unrefobj(lua_State* L,int ref);
void luaL_pushref(lua_State* L,int ref);

void luaL_openreg(lua_State* L,luaL_Reg* pTable,
	bool bMeta = false);

void luaL_registermeta(lua_State* L,const char* pName,
	luaL_Reg* pTable);
void luaL_pushmeta(lua_State* L,const char* pName);

#endif