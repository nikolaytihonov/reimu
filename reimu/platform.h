#ifndef __PLATFORM_H
#define __PLATFORM_H

typedef unsigned long ulong;

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE CHandle;
typedef LPTHREAD_START_ROUTINE ThreadStartFn;
typedef wchar_t U16Char;
typedef char U8Char;

typedef U16Char* U16String;
typedef U8Char* U8String;

#define MAX_TIMEOUT INFINITE
#define MAX_THREADS MAXIMUM_WAIT_OBJECTS
#else
#	error "Platform not supported yet"
#endif
#include <stdint.h>
#include <vector>

class CMutex
{
public:
	CMutex();
	~CMutex();

	void Lock();
	void Unlock();
private:
	CHandle m_hMutex;
};

class CThread
{
public:
	CThread();
	~CThread();

	void Start(ThreadStartFn ThreadFn,
		void* pArg);
	void Terminate(int iRet);
	void Close();

	static void WaitMulti(std::vector<CThread>& threads,
		bool bCloseAll,ulong ulTime = MAX_TIMEOUT);

	inline CHandle GetHandle(){
		return m_hThread;}
private:
	CHandle m_hThread;
};

typedef struct lua_State lua_State;

class CMemPool
{
public:
	CMemPool();
	~CMemPool();

	void Free();
	static size_t Writer(void* pData,size_t len,
		size_t memb,void* pArg);
	static int Dumper(lua_State* L,const void* pData,
		size_t len,void* B);

	void* m_pMem;
	size_t m_uLen;
};

class CConsole
{
public:
	CConsole();

	bool SetFont(U16String wFont,int iSize,
		bool bUnicode = true);

	void WriteW(U16String str,size_t uLen = 0);
	U16String ReadW(size_t uLen,size_t* uRLen);

	static U8String ToU8(U16String str,size_t uLen = 0);
	static U16String ToU16(U8String str,size_t uLen = 0);

	CHandle m_hInput;
	CHandle m_hOutput;
};

#endif