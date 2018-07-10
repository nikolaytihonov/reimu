#include "u8.h"
#include "utils.h"
#include "platform.h"

static CConsole s_Console;

static int printW(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	size_t uLen;
	DWORD dwTmp;

	LPWSTR pwBuf = (LPWSTR)lua_tolstring(L,1,&uLen);
	uLen/=sizeof(wchar_t);

	s_Console.WriteW(pwBuf,uLen);
	return 0;
}

static int scanW(lua_State* L)
{
	int iSize = 64;
	size_t uRead = 0;
	if(lua_type(L,1) == LUA_TNUMBER)
		iSize = lua_tointeger(L,1);
	LPWSTR pwBuf = s_Console.ReadW(iSize,&uRead);
	uRead-=2;
	lua_pushlstring(L,(const char*)pwBuf,
		uRead*sizeof(wchar_t));
	delete[] pwBuf;
	return 1;
}

static int writeW(lua_State* L)
{
	luaL_Stream* file = (luaL_Stream*)
		luaL_checkudata(L,1,LUA_FILEHANDLE);
	luaL_checktype(L,2,LUA_TSTRING);
	fprintf(file->f,"%s",lua_tostring(L,2));
	return 0;
}

static int conv_u16(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	size_t uLen;

	LPWSTR wStr = (LPWSTR)lua_tolstring(L,1,&uLen);
	char* pStr = CConsole::ToU8(wStr,uLen);
	lua_pushstring(L,pStr);
	delete[] pStr;

	return 1;
}

static int conv_u8(lua_State* L)
{
	luaL_checktype(L,1,LUA_TSTRING);
	size_t uLen,wLen;

	U8String pStr = (U8String)lua_tolstring(L,1,&uLen);
	LPWSTR wStr = CConsole::ToU16(pStr,uLen+1);
	wStr[(wLen = wcslen(wStr))] = 0;
	lua_pushlstring(L,(const char*)wStr,
		(wLen+1)*sizeof(wchar_t));
	delete[] wStr;

	return 1;
}

static luaL_Reg u8_funcs[] = {
	{"print",&printW},
	{"scan",&scanW},
	{"write",&writeW},
	{"conv_u16",&conv_u16},
	{"conv_u8",&conv_u8},
	{NULL,NULL}
};

void u8_init()
{
	s_Console.SetFont(L"Lucida Console",12);
}

int luaopen_u8(lua_State* L)
{
	luaL_openreg(L,u8_funcs);
	lua_setglobal(L,"u8");
	return 0;
}