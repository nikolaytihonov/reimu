#include "platform.h"

CMutex::CMutex()
{
	m_hMutex = CreateMutex(0,0,0);
}

CMutex::~CMutex()
{
	CloseHandle(m_hMutex);
}

void CMutex::Lock()
{
	WaitForSingleObject(m_hMutex,INFINITE);
}

void CMutex::Unlock()
{
	ReleaseMutex(m_hMutex);
}

CThread::CThread()
{
	m_hThread = NULL;
}

CThread::~CThread()
{
}

void CThread::Start(ThreadStartFn ThreadStart,
	void* pArg)
{
	m_hThread = CreateThread(0,0,ThreadStart,pArg,0,0);
}

void CThread::Terminate(int iRet)
{
	TerminateThread(m_hThread,iRet);
}

void CThread::Close()
{
	if(m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CThread::WaitMulti(std::vector<CThread>& threads,
	bool bCloseAll,ulong ulTime)
{
	CHandle* pHandles = new CHandle[threads.size()];
	for(size_t i = 0; i < threads.size(); i++)
		pHandles[i] = threads[i].GetHandle();
	WaitForMultipleObjects(threads.size(),pHandles,
		TRUE,ulTime);
	if(bCloseAll)
	{
		for each(CThread thr in threads)
			thr.Close();
		threads.clear();
	}
}

CMemPool::CMemPool()
{
	m_pMem = NULL;
	m_uLen = 0;
}

CMemPool::~CMemPool()
{
	Free();
}

void CMemPool::Free()
{
	if(m_pMem)
	{
		free(m_pMem);
		m_pMem = NULL;
		m_uLen = 0;
	}
}

size_t CMemPool::Writer(void* pData,size_t len,size_t memb,void* pArg)
{
	CMemPool* pMem = (CMemPool*)pArg;
	size_t sz = len*memb;
	if(!pMem->m_uLen) pMem->m_pMem = malloc(sz);
	else pMem->m_pMem = realloc(pMem->m_pMem,pMem->m_uLen+sz);
	memcpy((void*)((char*)pMem->m_pMem+pMem->m_uLen),pData,sz);
	pMem->m_uLen += sz;
	return sz;
}

int CMemPool::Dumper(lua_State *L, const void *data,
	size_t size, void *B) {
	CMemPool* pMem = (CMemPool*)B;
	if(!pMem->m_uLen) pMem->m_pMem = malloc(size);
	else pMem->m_pMem = realloc(pMem->m_pMem,
		pMem->m_uLen+size);
	memcpy((void*)((char*)pMem->m_pMem+pMem->m_uLen),
		data,size);
	pMem->m_uLen += size;
	return 0;
}

CConsole::CConsole()
{
	m_hInput = GetStdHandle(STD_INPUT_HANDLE);
	m_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool CConsole::SetFont(U16String wFont,int iSize,
	bool bUnicode)
{
	CONSOLE_FONT_INFOEX cfInfoEx;
	if(bUnicode);
		SetConsoleCP(CP_UTF8);

	cfInfoEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);

	cfInfoEx.dwFontSize.X = 0;
	cfInfoEx.dwFontSize.Y = iSize;

	wcscpy(cfInfoEx.FaceName,wFont);
	cfInfoEx.nFont = 0;
	cfInfoEx.FontWeight = FW_MEDIUM;
	cfInfoEx.FontFamily = FF_DONTCARE;

	return SetCurrentConsoleFontEx(m_hOutput,FALSE,&cfInfoEx);
}

void CConsole::WriteW(U16String wStr,size_t uLen)
{
	DWORD dwWritten = 0;
	if(!uLen) uLen = wcslen(wStr);
	WriteConsoleW(m_hOutput,wStr,uLen,
		&dwWritten,NULL);
}

U16String CConsole::ReadW(size_t uLen,
	size_t* uRLen)
{
	U16String wBuf = new U16Char[uLen];
	ReadConsoleW(m_hInput,wBuf,uLen,(LPDWORD)uRLen,
		NULL);
	return wBuf;
}

U8String CConsole::ToU8(U16String wStr,size_t uLen)
{
	if(!uLen)
		uLen = wcslen(wStr)*sizeof(U16Char);
	U8String pBuf = new U8Char[uLen];
	WideCharToMultiByte(CP_UTF8,0,wStr,
		wcslen(wStr),pBuf,uLen,0,FALSE);
	return pBuf;
}

U16String CConsole::ToU16(U8String pStr,size_t uLen)
{
	if(!uLen)
		uLen = strlen(pStr);
	U16String wBuf = new U16Char[uLen+1];
	MultiByteToWideChar(CP_UTF8,0,pStr,uLen,
		wBuf,(uLen*sizeof(U16Char)+1));
	return wBuf;
}