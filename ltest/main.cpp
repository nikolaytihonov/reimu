#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "lua.hpp"

int lSleep(lua_State* L)
{
	luaL_checktype(L,1,LUA_TNUMBER);
	Sleep((DWORD)lua_tointeger(L,1));
	return 0;
}

DWORD WINAPI LuaThread(LPVOID lpArg)
{
	lua_State* L = (lua_State*)lpArg;
	lua_getglobal(L,"callback");
	lua_call(L,0,1);
	int i = lua_tonumber(L,-1);
	lua_pop(L,1);

	printf("Returned: %d\n",i);
	return 0;
}

int main()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	lua_register(L,"sleep",lSleep);
	if(luaL_dofile(L,"main.lua"))
	{
		puts(lua_tostring(L,-1));
		lua_pop(L,1);
	}

	lua_State* pLuaThreads[10];
	HANDLE pThreads[10];
	for(int i = 0; i < 10; i++)
	{
		lua_State* NL = lua_newthread(L);
		pThreads[i] = CreateThread(0,0,LuaThread,(LPVOID)NL,0,0);
	}

	WaitForMultipleObjects(10,pThreads,TRUE,INFINITE);
	for(int i = 0; i < 10; i++)
	{
		//lua_close(pLuaThreads[i]);
		CloseHandle(pThreads[i]);
	}
	int iOld = lua_gc(L,LUA_GCCOUNT,0);
	lua_gc(L,LUA_GCCOLLECT,0);
	int iNew = lua_gc(L,LUA_GCCOUNT,0);

	printf("Collected: %d (KB)\n",iOld-iNew);
	lua_close(L);
	return 0;
}