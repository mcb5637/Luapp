# Luapp
C++ Lua bridge.
- Does not leak functions/structs into the global namespace (except lua_State and lua_Debug).
- Simple functions that should get inlined by any C++ compiler, giving you the same performance as if you were using the C API directly.
- Translating between Lua and C++ exceptions.
- Powerfull templates to create C++ classes into full userdata.
- Intended to be used as git submodule.

Usage:
- only use one version at a time
- make sure the following headers are includable: constexprTypename.h, luapp_common.h, luapp_decorator.h, luapp_userdata.h
- include the header of your version (example: luapp54.h for lua 5.4)
- add the cpp file of your version to compile (example: luapp54.cpp for lua 5.4)
- if you selected lua jit, also add luapp51.cpp and make sure, luapp51.h is includable
	(luapp51.cpp will include luajit instead of lua51 if lua51 is not found. it will default to lua51, if both are found, but they are ABI compatible, so using both jit or 51 is fine at that point)
- link against your lua lib/dll as usual
