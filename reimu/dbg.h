#ifndef __DBG_H
#define __DBG_H

#include "lua.hpp"
#include "platform.h"
#include <DbgHelp.h>

void dbg_init();
void dbg_makedump(struct _EXCEPTION_POINTERS*);

void dbg_errlog(const char* pFmt,...);

int luaopen_dbg(lua_State* L);

#endif