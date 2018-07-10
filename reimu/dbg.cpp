#include "dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef BOOL (WINAPI* MiniDumpWriteDump_t)(HANDLE,DWORD,HANDLE,
	MINIDUMP_TYPE,
	PMINIDUMP_EXCEPTION_INFORMATION,
	PMINIDUMP_USER_STREAM_INFORMATION,
	PMINIDUMP_CALLBACK_INFORMATION);

static MiniDumpWriteDump_t s_pMiniDumpWriteDump;

static LONG WINAPI _UEHandler(struct _EXCEPTION_POINTERS* pExcp)
{
	dbg_makedump(pExcp);
	dbg_errlog("UNHANDLED EXCEPTION %08X at %p\n",
		pExcp->ExceptionRecord->ExceptionCode,
		pExcp->ExceptionRecord->ExceptionAddress);

	CONTEXT ctx = *pExcp->ContextRecord;
	dbg_errlog("Registers:\nEAX %p EBX %p ECX %p EDX %p\n"
		"ESI %p EDI %p ESP %p EBP %p\nEIP %p EFLAGS %p\n",
		ctx.Eax,ctx.Ebx,ctx.Ecx,ctx.Edx,
		ctx.Esi,ctx.Edi,ctx.Esp,ctx.Ebp,
		ctx.Eip,ctx.EFlags);

	ExitProcess(pExcp->ExceptionRecord->ExceptionCode);
	return 0;
}

void dbg_errlog(const char* pFmt,...)
{
	char szBuf[256];
	FILE* log;
	va_list ap;

	va_start(ap,pFmt);
	vsnprintf_s(szBuf,256,pFmt,ap);
	va_end(ap);

	size_t uLen = strlen(szBuf);
	log = fopen("reimu.log","ab");
	fwrite(szBuf,uLen,1,log);
	fwrite(szBuf,uLen,1,stderr);

	SYSTEMTIME t;
	GetSystemTime(&t);

	fprintf(log,"[%02d/%02d/%04d %02d:%02d:%02d] %s",
		t.wDay,t.wMonth,t.wYear,t.wHour,
		t.wMinute,t.wSecond,szBuf);
	fclose(log);
}

void dbg_init()
{
	HMODULE hDbgHelp = NULL;
	if(!(hDbgHelp = LoadLibrary("DbgHelp.dll")))
	{
		dbg_errlog("Failed to load DbgHelp.dll!\n");
		return;
	}

	s_pMiniDumpWriteDump = (MiniDumpWriteDump_t)GetProcAddress(
		hDbgHelp,"MiniDumpWriteDump");

	SetUnhandledExceptionFilter(&_UEHandler);
}

void dbg_makedump(struct _EXCEPTION_POINTERS* pExcp)
{
	char szFileName[MAX_PATH];
	SYSTEMTIME t;
	GetSystemTime(&t);
	wsprintf(szFileName,"%02d-%02d-%04d_%02d-%02d-%02d_reimu.dmp",
		t.wDay,t.wMonth,t.wYear,t.wHour,t.wMinute,t.wSecond);

	HANDLE hFile = CreateFile(szFileName,GENERIC_ALL,FILE_SHARE_READ,
		NULL,CREATE_ALWAYS,0,NULL);
	if(!hFile)
	{
		dbg_errlog("Failed to alloc file %s for crash dump!\n",
			szFileName);
		return;
	}

	struct _MINIDUMP_EXCEPTION_INFORMATION exInfo;
	exInfo.ThreadId = GetCurrentThreadId();
	exInfo.ExceptionPointers = pExcp;
	exInfo.ClientPointers = FALSE;

	//Type = MiniDumpNormal|MiniDumpWithFullMemory|MiniDumpWithHandleData|

	if(!s_pMiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,
		MINIDUMP_TYPE(MiniDumpNormal|MiniDumpWithFullMemory|MiniDumpWithHandleData),
		pExcp?&exInfo:NULL,NULL,NULL))
		dbg_errlog("Failed to create dump (%d)\n",GetLastError());
	else printf("Dump %s created\n",szFileName);
	CloseHandle(hFile);
}

static int luaf_makedump(lua_State* L)
{
	dbg_makedump(NULL);
	return 0;
}

int luaopen_dbg(lua_State* L)
{
	lua_register(L,"dbg_makedump",&luaf_makedump);
	return 0;
}