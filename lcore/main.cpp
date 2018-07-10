#include <Windows.h>
#include <stdio.h>
#include "lua.hpp"
#include "html.h"

lua_State* g_pL;

int lcore_atpanic(lua_State* L)
{
	const char* pErr = lua_tostring(L,1);
	fprintf(stderr,"LUA PANIC: %s\n",pErr);
	return 0;
}

int lcore_aterror(lua_State* L)
{
	const char* pErr = lua_tostring(L,1);
	fprintf(stderr,"LUA ERROR: %s\n",pErr);
	return 0;
}

int main(int argc,char** argv)
{
	if(argc<2)
	{
		fprintf(stderr,"Usage: %s <script> <arg 1> <arg 2> ...\n",
			argv[0]);
		return 1;
	}

	g_pL = luaL_newstate();
	luaL_openlibs(g_pL);
	libhtml_open(g_pL);

	if(argc>2)
	{
		lua_newtable(g_pL);
		for(int i = 1; i < argc; i++)
		{
			lua_pushinteger(g_pL,i);
			lua_pushstring(g_pL,argv[i]);
			lua_settable(g_pL,-3);
		}
		lua_setglobal(g_pL,"args");
	}

	lua_pushcfunction(g_pL,lcore_aterror);
	if(luaL_loadfile(g_pL,argv[1]))
	{
		fprintf(stderr,"luaL_loadfile: %s\n",lua_tostring(g_pL,-1));
		lua_close(g_pL);
		return 1;
	}

	lua_pcall(g_pL,0,LUA_MULTRET,lua_gettop(g_pL)-1);
	lua_pop(g_pL,1);

	lua_gc(g_pL,LUA_GCCOLLECT,0);
	lua_close(g_pL);
	return 0;
}