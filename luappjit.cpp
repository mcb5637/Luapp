#include "../pch.h"

#include "luappjit_d.h"

#ifndef LUA_CPPLINKAGE
extern "C" {
#endif
#include "..\luajit\lua.h"
#include "..\luajit\lauxlib.h"
#include "..\luajit\lualib.h"
#include "..\luajit\luajit.h"
#ifndef LUA_CPPLINKAGE
}
#endif

#include <cstdlib>
#include <type_traits>
#include <sstream>

namespace lua::jit {
	// make sure all the constants match
	// i do define them new to avoid having to include the c lua files and having all their funcs/defines in global namespace
	static_assert(State::MINSTACK == LUA_MINSTACK);
	static_assert(LType::None == static_cast<LType>(LUA_TNONE));
	static_assert(LType::Nil == static_cast<LType>(LUA_TNIL));
	static_assert(LType::Number == static_cast<LType>(LUA_TNUMBER));
	static_assert(LType::Boolean == static_cast<LType>(LUA_TBOOLEAN));
	static_assert(LType::String == static_cast<LType>(LUA_TSTRING));
	static_assert(LType::Table == static_cast<LType>(LUA_TTABLE));
	static_assert(LType::Function == static_cast<LType>(LUA_TFUNCTION));
	static_assert(LType::Userdata == static_cast<LType>(LUA_TUSERDATA));
	static_assert(LType::Thread == static_cast<LType>(LUA_TTHREAD));
	static_assert(LType::LightUserdata == static_cast<LType>(LUA_TLIGHTUSERDATA));
	static_assert(std::is_same<Number, lua_Number>::value);
	static_assert(std::is_same<CFunction, lua_CFunction>::value);
	static_assert(std::is_same<CHook, lua_Hook>::value);
	static_assert(State::GLOBALSINDEX == LUA_GLOBALSINDEX);
	static_assert(State::ENVIRONINDEX == LUA_ENVIRONINDEX);
	static_assert(State::MULTIRET == LUA_MULTRET);
	static_assert(lua::v51::ErrorCode::Success == static_cast<lua::v51::ErrorCode>(0));
	static_assert(lua::v51::ErrorCode::Runtime == static_cast<lua::v51::ErrorCode>(LUA_ERRRUN));
	static_assert(lua::v51::ErrorCode::Memory == static_cast<lua::v51::ErrorCode>(LUA_ERRMEM));
	static_assert(lua::v51::ErrorCode::ErrorHandler == static_cast<lua::v51::ErrorCode>(LUA_ERRERR));
	static_assert(lua::v51::ErrorCode::Syntax == static_cast<lua::v51::ErrorCode>(LUA_ERRSYNTAX));
	static_assert(lua::v51::ErrorCode::File == static_cast<lua::v51::ErrorCode>(LUA_ERRFILE));
	static_assert(lua::v51::ErrorCode::Yield == static_cast<lua::v51::ErrorCode>(LUA_YIELD));
	static_assert(State::Upvalueindex(1) == lua_upvalueindex(1));
	static_assert(State::Upvalueindex(500) == lua_upvalueindex(500));
	static_assert(State::REGISTRYINDEX == LUA_REGISTRYINDEX);
	static_assert(lua::v51::DebugInfo::SHORTSRC_SIZE == LUA_IDSIZE);
	static_assert(lua::v51::HookEvent::None == static_cast<lua::v51::HookEvent>(0));
	static_assert(lua::v51::HookEvent::Call == static_cast<lua::v51::HookEvent>(LUA_MASKCALL));
	static_assert(lua::v51::HookEvent::Return == static_cast<lua::v51::HookEvent>(LUA_MASKRET));
	static_assert(lua::v51::HookEvent::Line == static_cast<lua::v51::HookEvent>(LUA_MASKLINE));
	static_assert(lua::v51::HookEvent::Count == static_cast<lua::v51::HookEvent>(LUA_MASKCOUNT));
	static_assert(JITMode::Off == static_cast<JITMode>(LUAJIT_MODE_OFF));
	static_assert(JITMode::On == static_cast<JITMode>(LUAJIT_MODE_ON));
	static_assert(JITMode::Flush == static_cast<JITMode>(LUAJIT_MODE_FLUSH));


	State::State(lua_State* L) : lua::v51::State(L) {
		static_assert(REFNILI == LUA_REFNIL);
		static_assert(NOREFI == LUA_NOREF);
	}
	State::State(bool io, bool debug) : lua::v51::State(luaL_newstate()) {
		lua_pushcfunction(L, &luaopen_base);
		lua_pushstring(L, "");
		lua_call(L, 1, 0);
		lua_pushcfunction(L, &luaopen_string);
		lua_pushstring(L, LUA_STRLIBNAME);
		lua_call(L, 1, 0);
		lua_pushcfunction(L, &luaopen_table);
		lua_pushstring(L, LUA_TABLIBNAME);
		lua_call(L, 1, 0);
		lua_pushcfunction(L, &luaopen_math);
		lua_pushstring(L, LUA_MATHLIBNAME);
		lua_call(L, 1, 0);
		lua_pushcfunction(L, &luaopen_bit);
		lua_pushstring(L, LUA_BITLIBNAME);
		lua_call(L, 1, 0);
		lua_pushcfunction(L, &luaopen_jit);
		lua_pushstring(L, LUA_JITLIBNAME);
		lua_call(L, 1, 0);
		if (io) {
			lua_pushcfunction(L, &luaopen_io);
			lua_pushstring(L, LUA_IOLIBNAME);
			lua_call(L, 1, 0);
			lua_pushcfunction(L, &luaopen_os);
			lua_pushstring(L, LUA_OSLIBNAME);
			lua_call(L, 1, 0);
			lua_pushcfunction(L, &luaopen_package);
			lua_pushstring(L, LUA_LOADLIBNAME);
			lua_call(L, 1, 0);
		}
		if (debug) {
			lua_pushcfunction(L, &luaopen_debug);
			lua_pushstring(L, LUA_DBLIBNAME);
			lua_call(L, 1, 0);
		}
		lua_settop(L, 0);
	}
	std::optional<Number> State::ToNumber(int index)
	{
		int isnum = 0;
		Number n = lua_tonumberx(L, index, &isnum);
		if (!isnum)
			return std::nullopt;
		return n;
	}
	std::optional<Integer> State::ToInteger(int index)
	{
		int isnum = 0;
		Integer n = lua_tointegerx(L, index, &isnum);
		if (!isnum)
			return std::nullopt;
		return n;
	}
	bool State::IsYieldable()
	{
		return lua_isyieldable(L);
	}
	Number State::Version() {
		return *lua_version(nullptr);
	}
	const void* State::Debug_UpvalueID(int index, int upnum)
	{
		return lua_upvalueid(L, index, upnum);
	}
	void State::Debug_UpvalueJoin(int funcMod, int upMod, int funcTar, int upTar)
	{
		lua_upvaluejoin(L, funcMod, upMod, funcTar, upTar);
	}

	void State::SetJITMode(JITMode m)
	{
		if (luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | static_cast<int>(m)) == 0)
			throw lua::LuaException{ "luaJIT_setmode failure" };
	}
	void State::SetJITModeForSingleFunc(int idx, JITMode m)
	{
		if (luaJIT_setmode(L, idx, LUAJIT_MODE_FUNC | static_cast<int>(m)) == 0)
			throw lua::LuaException{ "luaJIT_setmode failure" };
	}
	void State::SetJITModeForFuncAndChildren(int idx, JITMode m)
	{
		if (luaJIT_setmode(L, idx, LUAJIT_MODE_ALLFUNC | static_cast<int>(m)) == 0)
			throw lua::LuaException{ "luaJIT_setmode failure" };
	}
	void State::SetJITModeForChildrenOnly(int idx, JITMode m)
	{
		if (luaJIT_setmode(L, idx, LUAJIT_MODE_ALLSUBFUNC | static_cast<int>(m)) == 0)
			throw lua::LuaException{ "luaJIT_setmode failure" };
	}
};
