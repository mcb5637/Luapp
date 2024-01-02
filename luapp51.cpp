#include "../pch.h"

#include "luapp51.h"

#ifndef LUA_CPPLINKAGE
extern "C" {
#endif
#if __has_include("..\lua51\lua.h")
#include "..\lua51\lua.h"
#include "..\lua51\lauxlib.h"
#include "..\lua51\lualib.h"
#else
#include "..\luajit\lua.h"
#include "..\luajit\lauxlib.h"
#include "..\luajit\lualib.h"
#endif
#ifndef LUA_CPPLINKAGE
}
#endif

#include <cstdlib>
#include <type_traits>
#include <sstream>

namespace lua::v51 {
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
	static_assert(ErrorCode::Success == static_cast<ErrorCode>(0));
	static_assert(ErrorCode::Runtime == static_cast<ErrorCode>(LUA_ERRRUN));
	static_assert(ErrorCode::Memory == static_cast<ErrorCode>(LUA_ERRMEM));
	static_assert(ErrorCode::ErrorHandler == static_cast<ErrorCode>(LUA_ERRERR));
	static_assert(ErrorCode::Syntax == static_cast<ErrorCode>(LUA_ERRSYNTAX));
	static_assert(ErrorCode::File == static_cast<ErrorCode>(LUA_ERRFILE));
	static_assert(ErrorCode::Yield == static_cast<ErrorCode>(LUA_YIELD));
	static_assert(State::Upvalueindex(1) == lua_upvalueindex(1));
	static_assert(State::Upvalueindex(500) == lua_upvalueindex(500));
	static_assert(State::REGISTRYINDEX == LUA_REGISTRYINDEX);
	static_assert(DebugInfo::SHORTSRC_SIZE == LUA_IDSIZE);
	static_assert(HookEvent::None == static_cast<HookEvent>(0));
	static_assert(HookEvent::Call == static_cast<HookEvent>(LUA_MASKCALL));
	static_assert(HookEvent::Return == static_cast<HookEvent>(LUA_MASKRET));
	static_assert(HookEvent::Line == static_cast<HookEvent>(LUA_MASKLINE));
	static_assert(HookEvent::Count == static_cast<HookEvent>(LUA_MASKCOUNT));

	std::string(*ExceptionConverter)(std::exception_ptr ex, const char* funcsig) = nullptr;

	HookEvent LuaHookToEvent(int ev) {
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKCALL) == HookEvent::Call);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKRET) == HookEvent::Return);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKTAILRET) == HookEvent::TailReturn);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKLINE) == HookEvent::Line);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKCOUNT) == HookEvent::Count);
		return static_cast<HookEvent>(1 << ev);
	}
	void ClearDebug(lua_Debug& d) {
		d.event = 0;
		d.name = nullptr;
		d.namewhat = nullptr;
		d.what = nullptr;
		d.source = nullptr;
		d.currentline = 0;
		d.nups = 0;
		d.linedefined = 0;
		d.lastlinedefined = 0;
		d.short_src[0] = '\0';
	}
	void CopyDebugInfo(const lua_Debug& src, DebugInfo& trg) {
		trg.Event = LuaHookToEvent(src.event);
		trg.Name = src.name;
		trg.NameWhat = src.namewhat;
		trg.What = src.what;
		trg.Source = src.source;
		trg.CurrentLine = src.currentline;
		trg.NumUpvalues = src.nups;
		trg.LineDefined = src.linedefined;
		trg.LastLineDefined = src.lastlinedefined;
		memcpy(trg.ShortSrc, src.short_src, DebugInfo::SHORTSRC_SIZE);
		trg.ShortSrc[DebugInfo::SHORTSRC_SIZE - 1] = '\0';
	}

	State::State(lua_State* L)
	{
		static_assert(REFNILI == LUA_REFNIL);
		static_assert(NOREFI == LUA_NOREF);
		this->L = L;
	}

	State::State(bool io, bool debug)
	{
		L = luaL_newstate();
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

	lua_State* State::GetState()
	{
		return L;
	}
	void State::Close()
	{
		if (L != nullptr)
			lua_close(L);
		L = nullptr;
	}

	int State::GetTop()
	{
		return lua_gettop(L);
	}
	void State::SetTop(int index)
	{
		lua_settop(L, index);
	}
	void State::PushValue(int index)
	{
		lua_pushvalue(L, index);
	}
	void State::Remove(int index)
	{
		lua_remove(L, index);
	}
	void State::Insert(int index)
	{
		lua_insert(L, index);
	}
	void State::Replace(int index)
	{
		lua_replace(L, index);
	}
	void State::Copy(int from, int to)
	{
		to = ToAbsoluteIndex(to);
		PushValue(from);
		Replace(to);
	}
	void State::Pop(int num)
	{
		lua_pop(L, num);
	}
	LType State::Type(int index)
	{
		return static_cast<LType>(lua_type(L, index));
	}
	bool State::IsNil(int index)
	{
		return lua_isnil(L, index);
	}
	bool State::IsNone(int index)
	{
		return lua_isnone(L, index);
	}
	bool State::IsBoolean(int index)
	{
		return lua_isboolean(L, index);
	}
	bool State::IsNumber(int index)
	{
		return lua_isnumber(L, index);
	}
	bool State::IsString(int index)
	{
		return lua_isstring(L, index);
	}
	bool State::IsTable(int index)
	{
		return lua_istable(L, index);
	}
	bool State::IsFunction(int index)
	{
		return lua_isfunction(L, index);
	}
	bool State::IsCFunction(int index)
	{
		return lua_iscfunction(L, index);
	}
	bool State::IsUserdata(int index)
	{
		return lua_isuserdata(L, index);
	}
	bool State::IsLightUserdata(int index)
	{
		return lua_islightuserdata(L, index);
	}
	const char* State::TypeName(LType t)
	{
		return lua_typename(L, static_cast<int>(t));
	}
	int State::Compare_Unprotected(lua_State* L)
	{
		auto op = static_cast<ComparisonOperator>(static_cast<int>(lua_tointeger(L, 4)));
		bool r;
		switch (op)
		{
		case ComparisonOperator::Equals:
			r = lua_equal(L, 1, 2);
			break;
		case ComparisonOperator::LessThan:
			r = lua_lessthan(L, 1, 2);
			break;
		case ComparisonOperator::LessThanOrEquals:
			r = lua_equal(L, 1, 2) || lua_lessthan(L, 1, 2);
			break;
		default:
			r = false;
			break;
		}
		*static_cast<bool*>(lua_touserdata(L, 3)) = r;
		return 0;
	}
	bool State::RawEqual(int i1, int i2)
	{
		return lua_rawequal(L, i1, i2);
	}
	bool State::IsNoneOrNil(int idx)
	{
		return lua_isnoneornil(L, idx);
	}
	bool State::ToBoolean(int index)
	{
		return lua_toboolean(L, index);
	}
	std::optional<Number> State::ToNumber(int index)
	{
		Number n = lua_tonumber(L, index);
		if (n == 0 && !lua_isnumber(L, index))
			return std::nullopt;
		return n;
	}
	std::optional<Integer> State::ToInteger(int index)
	{
		Number n = lua_tonumber(L, index);
		if (n == 0 && !lua_isnumber(L, index))
			return std::nullopt;
		return static_cast<Integer>(n);
	}
	const char* State::ToString(int index, size_t* len)
	{
		return lua_tolstring(L, index, len);
	}
	CFunction State::ToCFunction(int index)
	{
		return lua_tocfunction(L, index);
	}
	State State::ToThread(int index)
	{
		lua_State* l = lua_tothread(L, index);
		if (!l)
			throw LuaException("invalid thread");
		return { l };
	}
	const void* State::ToPointer(int index)
	{
		return lua_topointer(L, index);
	}
	void* State::ToUserdata(int index)
	{
		return lua_touserdata(L, index);
	}
	size_t State::RawLength(int index)
	{
		return lua_objlen(L, index);
	}
	int State::ObjLen_Unprotected(lua_State* L)
	{
		luaL_dostring(L, "return function(a) return #a; end");
		lua_insert(L, -2);
		lua_call(L, 1, 1);
		return 1;
	}
	void State::Push(bool b)
	{
		lua_pushboolean(L, b);
	}
	void State::Push(Number n)
	{
		lua_pushnumber(L, n);
	}
	void State::Push(Integer i)
	{
		lua_pushnumber(L, static_cast<Number>(i));
	}
	void State::Push(const char* s)
	{
		lua_pushstring(L, s);
	}
	void State::Push(const char* s, size_t l)
	{
		lua_pushlstring(L, s, l);
	}
	void State::Push()
	{
		lua_pushnil(L);
	}
	void State::Push(CFunction f, int nups)
	{
		lua_pushcclosure(L, f, nups);
	}
	void State::PushLightUserdata(void* ud)
	{
		lua_pushlightuserdata(L, ud);
	}
	const char* State::PushVFString(const char* s, va_list argp)
	{
		return lua_pushvfstring(L, s, argp);
	}
	const char* State::PushFString(const char* s, ...)
	{
		va_list args;
		va_start(args, s);
		const char* r = PushVFString(s, args);
		va_end(args);
		return r;
	}
	int State::Concat_Unprotected(lua_State* L)
	{
		int n = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_concat(L, n);
		return 1;
	}
	int State::Arithmetic_Unprotected(lua_State* L)
	{
		ArihmeticOperator op = static_cast<ArihmeticOperator>(static_cast<int>(lua_tonumber(L, -1)));
		lua_pop(L, 1);
		switch (op)
		{
		case ArihmeticOperator::Add:
			luaL_dostring(L, "return function(a, b) return a + b; end");
			break;
		case ArihmeticOperator::Subtract:
			luaL_dostring(L, "return function(a, b) return a - b; end");
			break;
		case ArihmeticOperator::Multiply:
			luaL_dostring(L, "return function(a, b) return a * b; end");
			break;
		case ArihmeticOperator::Divide:
			luaL_dostring(L, "return function(a, b) return a / b; end");
			break;
		case ArihmeticOperator::Modulo:
			luaL_dostring(L, "return function(a, b) return a % b; end");
			break;
		case ArihmeticOperator::Pow:
			luaL_dostring(L, "return function(a, b) return a ^ b; end");
			break;
		case ArihmeticOperator::UnaryNegation:
			luaL_dostring(L, "return function(a) return -a; end");
			break;
		default:
			lua_pushnil(L);
		}
		lua_insert(L, op == ArihmeticOperator::UnaryNegation ? -2 : -3);
		lua_call(L, op == ArihmeticOperator::UnaryNegation ? 1 : 2, 1);
		return 1;
	}
	bool State::GetMetatable(int index)
	{
		return lua_getmetatable(L, index);
	}
	bool State::SetMetatable(int index)
	{
		return lua_setmetatable(L, index);
	}
	void* State::NewUserdata(size_t s)
	{
		return lua_newuserdata(L, s);
	}
	LType State::GetUserValue(int index)
	{
		lua_getfenv(L, index);
		return Type(-1);
	}
	void State::SetUserValue(int index)
	{
		lua_setfenv(L, index);
	}
	ErrorCode State::Load(const char* (__cdecl* reader)(lua_State*, void*, size_t*), void* ud, const char* chunkname)
	{
		return static_cast<ErrorCode>(lua_load(L, reader, ud, chunkname));
	}
	void State::Dump(int(__cdecl* writer)(lua_State*, const void*, size_t, void*), void* ud)
	{
		lua_dump(L, writer, ud);
	}
	void State::NewTable()
	{
		lua_newtable(L);
	}
	int State::GetTable_Unprotected(lua_State* L)
	{
		lua_gettable(L, 1);
		return 1;
	}
	void State::GetTableRaw(int index)
	{
		if constexpr (TypeChecks) {
			if (Type(index) != LType::Table)
				throw lua::LuaException{ "GetTableRaw cannot access non tables" };
			CheckStackHasElements(IsPseudoIndex(index) ? 1 : 2);
		}
		lua_rawget(L, index);
	}
	void State::GetTableRaw(int index, int n)
	{
		if constexpr (TypeChecks) {
			if (Type(index) != LType::Table)
				throw lua::LuaException{ "GetTableRaw cannot access non tables" };
		}
		lua_rawgeti(L, index, n);
	}
	int State::SetTable_Unprotected(lua_State* L)
	{
		lua_settable(L, 1);
		return 0;
	}
	void State::SetTableRaw(int index)
	{
		if constexpr (TypeChecks) {
			if (Type(index) != LType::Table)
				throw lua::LuaException{ "SetTableRaw cannot access non tables" };
			CheckStackHasElements(IsPseudoIndex(index) ? 2 : 3);
		}
		lua_rawset(L, index);
	}
	void State::SetTableRaw(int index, int n)
	{
		if constexpr (TypeChecks) {
			if (Type(index) != LType::Table)
				throw lua::LuaException{ "SetTableRaw cannot access non tables" };
			CheckStackHasElements(IsPseudoIndex(index) ? 1 : 2);
		}
		lua_rawseti(L, index, n);
	}
	void State::PushGlobalTable()
	{
		PushValue(GLOBALSINDEX);
	}
	int State::Next_Unproteced(lua_State* L)
	{
		bool has = lua_next(L, 2);
		*static_cast<bool*>(lua_touserdata(L, 1)) = has;
		return has ? 2 : 0;
	}
	void State::GetEnvironment(int idx)
	{
		lua_getfenv(L, idx);
	}
	bool State::SetEnvironment(int idx)
	{
		return lua_setfenv(L, idx);
	}
	void State::Call(int nargs, int nresults)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(nargs + 1);
		}
		lua_call(L, nargs, nresults);
	}
	ErrorCode State::PCall(int nargs, int nresults, int errfunc)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(nargs + 1 + (errfunc == 0 ? 0 : 1));
		}
		return static_cast<ErrorCode>(lua_pcall(L, nargs, nresults, errfunc));
	}
	const char* State::ErrorCodeFormat(ErrorCode c)
	{
		switch (c)
		{
		case ErrorCode::Success:
			return "Lua_Success: ";
		case ErrorCode::Runtime:
			return "Lua_RuntimeError: ";
		case ErrorCode::File:
			return "Lua_FileError: ";
		case ErrorCode::Syntax:
			return "Lua_SyntaxError: ";
		case ErrorCode::Memory:
			return "Lua_MemoryError: ";
		case ErrorCode::ErrorHandler:
			return "Lua_HandlerError: ";
		default:
			return "Lua_UnknownErrorCode: ";
		}
	}
	void State::Error()
	{
		lua_error(L);
	}
	State State::NewThread()
	{
		return { lua_newthread(L) };
	}
	ErrorCode State::ResumeThread(int narg)
	{
		if constexpr (TypeChecks) {
			if (Type(-narg - 1) != LType::Thread)
				throw lua::LuaException{ "ResumeThread trying to resume non thread" };
		}
		return static_cast<ErrorCode>(lua_resume(L, narg));
	}
	void State::YieldThread(int nret)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(nret);
		}
		lua_yield(L, nret);
	}
	void State::XMove(State to, int num)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(num);
		}
		lua_xmove(L, to.L, num);
	}
	Number State::Version() {
		return LUA_VERSION_NUM;
	}
	constexpr const char* Debug_GetOptionString(DebugInfoOptions opt, bool pushFunc, bool fromStack)
	{
		if (fromStack) {
			switch (opt) {
			case DebugInfoOptions::None:
				return ">";
			case DebugInfoOptions::Name:
				return ">n";
			case DebugInfoOptions::Source:
				return ">S";
			case DebugInfoOptions::Source | DebugInfoOptions::Name:
				return ">Sn";
			case DebugInfoOptions::Line:
				return ">l";
			case DebugInfoOptions::Line | DebugInfoOptions::Name:
				return ">ln";
			case DebugInfoOptions::Line | DebugInfoOptions::Source:
				return ">lS";
			case DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return ">lSn";
			case DebugInfoOptions::Upvalues:
				return ">u";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Name:
				return ">un";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source:
				return ">uS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return ">uSn";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line:
				return ">ul";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Name:
				return ">uln";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source:
				return ">ulS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return ">ulSn";
			default:
				return "";
			}
		}
		else if (pushFunc) {
			switch (opt) {
			case DebugInfoOptions::None:
				return "f";
			case DebugInfoOptions::Name:
				return "fn";
			case DebugInfoOptions::Source:
				return "fS";
			case DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "fSn";
			case DebugInfoOptions::Line:
				return "fl";
			case DebugInfoOptions::Line | DebugInfoOptions::Name:
				return "fln";
			case DebugInfoOptions::Line | DebugInfoOptions::Source:
				return "flS";
			case DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "flSn";
			case DebugInfoOptions::Upvalues:
				return "fu";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Name:
				return "fun";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source:
				return "fuS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "fuSn";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line:
				return "ful";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Name:
				return "fuln";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source:
				return "fulS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "fulSn";
			default:
				return "";
			}
		}
		else {

			switch (opt) {
			case DebugInfoOptions::None:
				return "";
			case DebugInfoOptions::Name:
				return "n";
			case DebugInfoOptions::Source:
				return "S";
			case DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "Sn";
			case DebugInfoOptions::Line:
				return "l";
			case DebugInfoOptions::Line | DebugInfoOptions::Name:
				return "ln";
			case DebugInfoOptions::Line | DebugInfoOptions::Source:
				return "lS";
			case DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "lSn";
			case DebugInfoOptions::Upvalues:
				return "u";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Name:
				return "un";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source:
				return "uS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "uSn";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line:
				return "ul";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Name:
				return "uln";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source:
				return "ulS";
			case DebugInfoOptions::Upvalues | DebugInfoOptions::Line | DebugInfoOptions::Source | DebugInfoOptions::Name:
				return "ulSn";
			default:
				return "";
			}
		}
	}
	bool State::Debug_IsStackLevelValid(int lvl)
	{
		lua_Debug d;
		return lua_getstack(L, lvl, &d);
	}
	bool State::Debug_GetStack(int level, DebugInfo& Info, DebugInfoOptions opt, bool pushFunc)
	{
		lua_Debug d;
		ClearDebug(d);
		if (!lua_getstack(L, level, &d))
			return false;
		if (!lua_getinfo(L, Debug_GetOptionString(opt, pushFunc, false), &d))
			throw std::runtime_error("somehow the debug option string got messed up");
		CopyDebugInfo(d, Info);
		return true;
	}
	DebugInfo State::Debug_GetInfoForFunc(DebugInfoOptions opt)
	{
		lua_Debug d;
		ClearDebug(d);
		DebugInfo r{};
		if (!lua_getinfo(L, Debug_GetOptionString(opt, false, true), &d))
			throw std::runtime_error("somehow the debug option string got messed up");
		CopyDebugInfo(d, r);
		return r;
	}
	const char* State::Debug_GetLocal(int level, int localnum)
	{
		lua_Debug ar;
		if (!lua_getstack(L, level, &ar))
			return nullptr;
		return lua_getlocal(L, &ar, localnum);
	}
	const char* State::Debug_SetLocal(int level, int localnum)
	{
		lua_Debug ar;
		if (!lua_getstack(L, level, &ar))
			return nullptr;
		return lua_setlocal(L, &ar, localnum);
	}
	const char* State::Debug_GetUpvalue(int index, int upnum)
	{
		return lua_getupvalue(L, index, upnum);
	}
	const char* State::Debug_SetUpvalue(int index, int upnum)
	{
		return lua_setupvalue(L, index, upnum);
	}
	void State::Debug_SetHook(CHook hook, HookEvent mask, int count)
	{
		lua_sethook(L, hook, static_cast<int>(mask), count);
	}
	void State::Debug_UnSetHook()
	{
		lua_sethook(L, nullptr, 0, 0);
	}
	DebugInfo State::Debug_GetInfoFromAR(ActivationRecord ar, DebugInfoOptions opt, bool pushFunc)
	{
		DebugInfo r{};
		if (!lua_getinfo(L, Debug_GetOptionString(opt, pushFunc, false), ar.ar))
			throw std::runtime_error("ActivationRecord no longer valid");
		CopyDebugInfo(*ar.ar, r);
		return r;
	}
	CHook State::Debug_GetHook()
	{
		return lua_gethook(L);
	}
	HookEvent State::Debug_GetHookMask()
	{
		return static_cast<HookEvent>(lua_gethookmask(L));
	}
	int State::Debug_GetHookCount()
	{
		return lua_gethookcount(L);
	}
	void State::CheckStackHasElements(int n)
	{
		int t = GetTop();
		if (t < n)
			throw lua::LuaException{ "not enough stack elements" };
	}
	ErrorCode State::DoFile(const char* filename)
	{
		return static_cast<ErrorCode>(luaL_dofile(L, filename));
	}
	ErrorCode State::DoString(const char* code)
	{
		return static_cast<ErrorCode>(luaL_dostring(L, code));
	}
	ErrorCode State::DoString(const char* code, size_t l, const char* name)
	{
		return static_cast<ErrorCode>(luaL_loadbuffer(L, code, l, name) || lua_pcall(L, 0, LUA_MULTRET, 0));
	}
	ErrorCode State::LoadBuffer(const char* code, size_t len, const char* name)
	{
		return static_cast<ErrorCode>(luaL_loadbuffer(L, code, len, name));
	}
	ErrorCode State::LoadFile(const char* filename)
	{
		return static_cast<ErrorCode>(luaL_loadfile(L, filename));
	}
	int State::RefI(int t)
	{
		return luaL_ref(L, t);
	}
	void State::UnRefI(int r, int t)
	{
		luaL_unref(L, t, r);
	}
	bool State::CheckStack(int extra)
	{
		return lua_checkstack(L, extra);
	}
	bool State::IsValidIndex(int i)
	{
		return 1 <= std::abs(i) && std::abs(i) <= GetTop();
	}
	int State::ToAbsoluteIndex(int i)
	{
		if (i > 0)
			return i;
		if (IsPseudoIndex(i))
			return i;
		return GetTop() + i + 1;
	}
	ActivationRecord::ActivationRecord(lua_Debug* ar)
	{
		this->ar = ar;
	}
	HookEvent ActivationRecord::Event() const
	{
		return LuaHookToEvent(ar->event);
	}
	int ActivationRecord::Line() const
	{
		return ar->currentline;
	}
	bool ActivationRecord::Matches(HookEvent e) const
	{
		return (Event() & e) != HookEvent::None;
	}
};
