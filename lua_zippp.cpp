#define LUA_LIB
#define LUA_BUILD_AS_DLL



#include "lua.hpp"
#pragma comment(lib, "lua.lib")

#pragma once
#define DllExport   __declspec( dllexport )//宏定义
#ifndef zippp_H
#define zippp_H

class DllExport zippp
{

};

#endif

static int sum(lua_State* L) {
	int num1 = lua_tointeger(L, 1);
	int num2 = lua_tointeger(L, 2);
	int result;
	result = num1 + num2;
	lua_pushnumber(L, result);
	return 1;
}

static struct luaL_Reg ls_lib[] = {
	{"sum", sum},
	{nullptr, nullptr}
};

extern "C" LUALIB_API int luaopen_lua_zippp(lua_State * L) {
#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, ls_lib);
#else
	luaL_openlib(L, "lua_zippp", ls_lib, 0);
#endif

	return 1;
}

extern "C" {
	int sum(int x, int y) {
		return x + y;
	}
}