#ifndef __LCURL_H
#define __LCURL_H

#include "curl/curl.h"
#include "lua.hpp"

typedef struct {
	CURL* m_pCurl;
} lcurl_t;

int luaopen_curl(lua_State* L);

#endif