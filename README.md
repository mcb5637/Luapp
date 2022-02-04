# Luapp
C++ Lua bridge.
- Does not leak functions/structs into the global namespace (except lua_State and lua_Debug).
- Simple functions that should get inlined by any C++ compiler, giving you the same performance as if you were using the C API directly.
- Translating between Lua and C++ exceptions.
- Powerfull templates to create C++ classes into full userdata.
- Intended to be used as git submodule.
