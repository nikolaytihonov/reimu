#include <stdio.h>
#include <stdlib.h>
#include "lua.hpp"
#include "html.h"
#include "lcurl.h"
#include "task.h"
#include "lcurl.h"
#include "file.h"
#include "u8.h"
#include "dbg.h"

lua_State* gL;

int g_iArgc;
char** g_ppArgv;

static void dumpstack(lua_State* L)
{
	int iTop = lua_gettop(L);
	for(int i = 1; i <= iTop; i++)
	{
		const char* pType = lua_typename(
			L,lua_type(L,i));
		const char* pValue = lua_tostring(L,i);
		dbg_errlog("%d: %s (%s)\n",i,
			pValue?pValue:"(null)",
			pType?pType:"(null)");
	}
}

int reimu_panic(lua_State* L)
{
	dbg_errlog("PANIC: %s\n",lua_tostring(L,1));
	dbg_makedump(NULL);
	return 0;
}

int reimu_error(lua_State* L)
{
	dumpstack(L);
	luaL_traceback(L,L,NULL,1);
	dbg_errlog("%s: %s\n%s\n",(L==gL)?"FATAL ERROR":"ERROR",
		lua_tostring(L,1),lua_tostring(L,-1));
	lua_pop(L,1);
	return 0;
}

void reimu_openlibs(lua_State* L)
{
	luaL_openlibs(L);
	luaopen_html(L);
	luaopen_curl(L);
	luaopen_task(L);
	luaopen_file(L);
	luaopen_u8(L);
	luaopen_dbg(L);

	if(g_iArgc>=2)
	{
		lua_newtable(L);
		for(int i = 1; i <= g_iArgc; i++)
		{
			lua_pushstring(L,g_ppArgv[i]);
			lua_seti(L,-2,i);
		}
		lua_setglobal(L,"args");
	}
}

int main(int argc,char** argv)
{
	int iOldGC,iNewGC;

	if(argc<2)
	{
		dbg_errlog("Usage: %s <script> <arg 1> <arg 2> ...\n",
			argv[0]);
		return 1;
	}

	dbg_init();
	u8_init();

	g_iArgc = argc;
	g_ppArgv = argv;

	gL = luaL_newstate();
	reimu_openlibs(gL);

	lua_atpanic(gL,reimu_panic);

	lua_pushcfunction(gL,reimu_error);
	if(luaL_loadfile(gL,argv[1]))
	{
		dbg_errlog("luaL_loadfile: %s\n",
			lua_tostring(gL,-1));
		lua_pop(gL,1);
	}
	else lua_pcall(gL,0,LUA_MULTRET,lua_gettop(gL)-1);
	lua_pop(gL,1);
	
	iOldGC = lua_gc(gL,LUA_GCCOUNT,0);
	lua_gc(gL,LUA_GCCOLLECT,0);
	iNewGC = lua_gc(gL,LUA_GCCOUNT,0);

	printf("Collected: %d (KB)\n",iOldGC-iNewGC);
	lua_close(gL);
	return 0;
}