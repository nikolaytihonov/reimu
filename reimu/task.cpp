#include "task.h"
#include "platform.h"
#include <string.h>
#include <malloc.h>
#include "lcurl.h"
#include "utils.h"
#include "dbg.h"

static CMutex s_TaskMutex;

static void luaf_dumpstack(lua_State* L)
{
	int iTop = lua_gettop(L);
	for(int i = iTop; i > 0; i--)
		printf("[%d] %s\n",i,lua_typename(L,lua_type(L,i)));
}

DWORD WINAPI LuaThread(LPVOID lpArg)
{
	lua_State* L = (lua_State*)lpArg;

	//luaf_dumpstack(L);
	if(lua_pcall(L,1,0,lua_gettop(L)-2))
		lua_pop(L,1);
	lua_pop(L,1);

	lua_close(L);
	return 0;
}

/*
Arguments:
1 - Thread function, 2 - Table, which represents number
of threads and his arguments
Return: nothing?
*/
int c_iBlockSize = MAX_THREADS;
int c_iDelay = 0;
ulong c_ulTimeOut = MAX_TIMEOUT;

extern int reimu_error(lua_State* L);
extern void reimu_openlibs(lua_State* L);

static void copy_value(lua_State* Ldst,lua_State* Lsrc,
	int n)
{
	int iType = lua_type(Lsrc,n);
	switch(iType)
	{
	case LUA_TNUMBER:
		lua_pushnumber(Ldst,lua_tonumber(Lsrc,n));
		break;
	case LUA_TSTRING:
		lua_pushstring(Ldst,lua_tostring(Lsrc,n));
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(Ldst,lua_toboolean(Lsrc,n));
		break;
	default:
		luaL_error(Lsrc,"copy_value type not supported (%s)",
			lua_typename(Lsrc,iType));
	}
}

/*static void copy_function(lua_State* Ldst,lua_State* Lsrc,
	int n)
{
	mempool_t mem;
	mem.m_pData = NULL;
	mem.m_uLen = 0;

	lua_pushvalue(Lsrc,n);
	if(lua_dump(Lsrc,mempool_writer,&mem,0))
	{
		//luaL_error(Lsrc,"failed to dump function");
		luaL_error(Lsrc,"DUMP: %s",lua_tostring(Lsrc,-1));
		lua_pop(Lsrc,1);
		return;
	}
	lua_pop(Lsrc,1);
	if(luaL_loadbuffer(Ldst,(const char*)mem.m_pData,mem.m_uLen,
		"thread_func"))
	{
		luaL_error(Ldst,lua_tostring(Ldst,-1));
		lua_pop(Ldst,1);
	}
}*/

inline bool dump_function(lua_State* L,int idx,CMemPool* pFunc)
{
	lua_pushvalue(L,idx);
	if(lua_dump(L,CMemPool::Dumper,pFunc,0))
	{
		luaL_error(L,"DUMP: %s",lua_tostring(L,-1));
		lua_pop(L,2);
		return false;
	}
	lua_pop(L,1);
	return true;
}

inline bool load_function(lua_State* L,CMemPool* pFunc)
{
	if(luaL_loadbuffer(L,(const char*)pFunc->m_pMem,
		pFunc->m_uLen,"thread_func"))
	{
		luaL_error(L,"LOAD: %s",lua_tostring(L,-1));
		lua_pop(L,1);
		return false;
	}
	return true;
}

static lua_State* _init_state(lua_State* L,CMemPool* pFunc,
	int iArgTable,int iThrArg)
{
	lua_State* pL = luaL_newstate();
	reimu_openlibs(pL);

	if(!load_function(pL,pFunc))
	{
		lua_close(pL);
		return NULL;
	}

	s_TaskMutex.Lock();
	lua_rawgeti(L,iArgTable,iThrArg);
	if(lua_isnil(L,-1))
	{
		dbg_errlog("Unsupported value at %d\n",
			iThrArg);
		lua_close(pL);
		lua_pop(L,1);

		s_TaskMutex.Unlock();
		return NULL;
	}

	copy_value(pL,L,-1);
	lua_pop(L,1);
	s_TaskMutex.Unlock();
	
	return pL;
}

static int performMultiTask(lua_State* L)
{
	luaL_checktype(L,1,LUA_TFUNCTION);
	luaL_checktype(L,2,LUA_TTABLE);

	int iThreadNum = lua_rawlen(L,2);
	int iBlocks = iThreadNum/c_iBlockSize,
		iLast = iThreadNum%c_iBlockSize;
	int iThrIdx = 0;

	lua_State** pL = new lua_State*[c_iBlockSize];
	std::vector<CThread> threads;
	CMemPool lFunc;

	if(!dump_function(L,1,&lFunc))
		return 0;

	for(int m = 0; m < iBlocks; m++)
	{
		for(int i = 0; i < c_iBlockSize; i++)
		{
			iThrIdx = (m*c_iBlockSize)+i;
			if(!(pL[i] = _init_state(L,&lFunc,
				2,iThrIdx+1)))
			{
				dbg_errlog("performMultiTask: _init_state thr fail %d\n",
					iThrIdx);
				continue;
			}

			CThread thr = CThread();
			thr.Start(LuaThread,pL[i]);
			threads.push_back(thr);
		}

		CThread::WaitMulti(threads,true,c_ulTimeOut);
		if(c_iDelay) Sleep(c_iDelay);
	}

	if(iLast)
	{
		for(int i = 0; i < iLast; i++)
		{
			iThrIdx = (iBlocks*c_iBlockSize)+i;
			if(!(pL[i] = _init_state(L,&lFunc,
				2,iThrIdx+1)))
			{
				dbg_errlog("performMultiTask: _init_state thr fail %d\n",
					iThrIdx);
				continue;
			}

			CThread thr = CThread();
			thr.Start(LuaThread,pL[i]);
			threads.push_back(thr);
		}
		CThread::WaitMulti(threads,true,c_ulTimeOut);
	}
	return 0;
}

static int task_setThreadCount(lua_State* L)
{
	luaL_checktype(L,1,LUA_TNUMBER);
	s_TaskMutex.Lock();
	c_iBlockSize = (int)lua_tonumber(L,1);
	s_TaskMutex.Unlock();
	return 0;
}

static int task_setDelay(lua_State* L)
{
	luaL_checktype(L,1,LUA_TNUMBER);
	s_TaskMutex.Lock();
	c_iDelay = (int)lua_tonumber(L,1);
	s_TaskMutex.Unlock();
	return 0;
}

static int task_setTimeOut(lua_State* L)
{
	luaL_checktype(L,1,LUA_TNUMBER);
	s_TaskMutex.Lock();
	c_ulTimeOut = (ulong)lua_tonumber(L,1);
	s_TaskMutex.Unlock();
	return 0;
}

static int task_lockGlobal(lua_State* L)
{
	s_TaskMutex.Lock();
	return 0;
}

static int task_unlockGlobal(lua_State* L)
{
	s_TaskMutex.Unlock();
	return 0;
}

extern lua_State* gL;

static int task_getGlobal(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	s_TaskMutex.Lock();
	lua_getglobal(gL,lua_tostring(L,1));
	copy_value(L,gL,-1);
	lua_pop(gL,1);
	s_TaskMutex.Unlock();
	return 1;
}

static luaL_Reg task_funcs[] = {
	{"setThreadCount",&task_setThreadCount},
	{"setDelay",&task_setDelay},
	{"setTimeOut",&task_setTimeOut},
	{"getGlobal",&task_getGlobal},
	{"lockGlobal",&task_lockGlobal},
	{"unlockGlobal",&task_unlockGlobal},
	{NULL,NULL}
};

int luaopen_task(lua_State* L)
{
	lua_register(L,"performMultiTask",performMultiTask);
	luaL_openreg(L,task_funcs);
	lua_setglobal(L,"task");
	return 0;
}