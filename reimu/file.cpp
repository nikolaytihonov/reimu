#include "file.h"
#include "utils.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PUSH_FIELD(L,st,field)	\
	lua_pushnumber(L,st.field);	\
	lua_setfield(L,-2,#field);

static int file_stat(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	struct stat st;
	if(stat(lua_tostring(L,1),&st)<0)
		lua_pushnil(L);
	else
	{
		lua_newtable(L);
		
		PUSH_FIELD(L,st,st_dev);
		PUSH_FIELD(L,st,st_ino);
		PUSH_FIELD(L,st,st_mode);
		PUSH_FIELD(L,st,st_nlink);
		PUSH_FIELD(L,st,st_uid);
		PUSH_FIELD(L,st,st_gid);
		PUSH_FIELD(L,st,st_rdev);
		PUSH_FIELD(L,st,st_size);
		PUSH_FIELD(L,st,st_atime);
		PUSH_FIELD(L,st,st_mtime);
		PUSH_FIELD(L,st,st_ctime);
	}
	return 1;
}

static int file_size(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	struct stat st;
	if(stat(lua_tostring(L,1),&st)<0)
		lua_pushnil(L);
	else lua_pushnumber(L,st.st_size);
	return 1;
}

static int file_exists(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	struct stat st;
	lua_pushboolean(L,(stat(lua_tostring(L,1),&st)!=-1));
	return 1;
}

static int file_mkdir(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	lua_pushboolean(L,CreateDirectory(
		lua_tostring(L,1),NULL));
	return 1;
}

static int file_remove(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	lua_pushinteger(L,remove(
		lua_tostring(L,1)));
	return 1;
}

static luaL_Reg file_funcs[] = {
	{"exists",&file_exists},
	{"mkdir",&file_mkdir},
	{"stat",&file_stat},
	{"size",&file_size},
	{"remove",&file_remove},
	{NULL,NULL}
};

static char* s_pMutexName = "mutex";

CMutex** luaf_makemutex(lua_State* L)
{
	CMutex** ppMutex = (CMutex**)lua_newuserdata(L,
		sizeof(CMutex*));
	*ppMutex = new CMutex();
	luaL_pushmeta(L,s_pMutexName);
	lua_setmetatable(L,-2);
	return ppMutex;
}

static int mutex_tostring(lua_State* L)
{
	CMutex** ppMutex = luaf_checkud<CMutex*>(L,
		1);
	lua_pushfstring(L,"%s %p",s_pMutexName,
		*ppMutex);
	return 1;
}

static int mutex_gc(lua_State* L)
{
	CMutex** ppMutex = luaf_checkud<CMutex*>(L,
		1);
	if(*ppMutex)
	{
		delete *ppMutex;
		*ppMutex = NULL;
	}
	return 0;
}

static int mutex_lock(lua_State* L)
{
	CMutex** ppMutex = luaf_checkud<CMutex*>(L,
		1);
	if(*ppMutex) (*ppMutex)->Lock();
	return 0;
}

static int mutex_unlock(lua_State* L)
{
	CMutex** ppMutex = luaf_checkud<CMutex*>(L,
		1);
	if(*ppMutex) (*ppMutex)->Unlock();
	return 0;
}

static luaL_Reg mutex_funcs[] = {
	{"__tostring",&mutex_tostring},
	{"__gc",&mutex_gc},
	{"lock",&mutex_lock},
	{"unlock",&mutex_unlock},
	{NULL,NULL}
};

static int mutex_open(lua_State* L)
{
	luaf_makemutex(L);
	return 1;
}

int luaopen_file(lua_State* L)
{
	luaL_openreg(L,file_funcs);
	lua_setglobal(L,"file");

	luaL_registermeta(L,mutex_funcs,s_pMutexName);
	lua_register(L,"mutex_open",mutex_open);
	return 0;
}