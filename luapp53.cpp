#include "luapp53_d.h"
#include <cstring>

#ifndef LUA_CPPLINKAGE
extern "C" {
#endif
#include "../lua53/lua.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#include "../lua53/lauxlib.h"
#pragma clang diagnostic pop
#include "../lua53/lualib.h"
#ifndef LUA_CPPLINKAGE
}
#endif

#include <cstdlib>
#include <sstream>

#include "lookup_table.h"

namespace lua::v53 {
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
	static_assert(std::same_as<Number, lua_Number>);
	static_assert(std::same_as<Integer, lua_Integer>);
	static_assert(std::same_as<CFunction, lua_CFunction>);
	static_assert(std::same_as<CHook, lua_Hook>);
	static_assert(State::MULTIRET == LUA_MULTRET);
	static_assert(ErrorCode::Success == static_cast<ErrorCode>(0));
	static_assert(ErrorCode::Runtime == static_cast<ErrorCode>(LUA_ERRRUN));
	static_assert(ErrorCode::Memory == static_cast<ErrorCode>(LUA_ERRMEM));
	static_assert(ErrorCode::ErrorHandler == static_cast<ErrorCode>(LUA_ERRERR));
	static_assert(ErrorCode::Syntax == static_cast<ErrorCode>(LUA_ERRSYNTAX));
	static_assert(ErrorCode::File == static_cast<ErrorCode>(LUA_ERRFILE));
	static_assert(ErrorCode::Yield == static_cast<ErrorCode>(LUA_YIELD));
	static_assert(ErrorCode::GarbageCollection == static_cast<ErrorCode>(LUA_ERRGCMM));
	static_assert(State::Upvalueindex(1) == lua_upvalueindex(1));
	static_assert(State::Upvalueindex(500) == lua_upvalueindex(500));
	static_assert(State::REGISTRYINDEX == LUA_REGISTRYINDEX);
	static_assert(DebugInfo::SHORTSRC_SIZE == LUA_IDSIZE);
	static_assert(HookEvent::None == static_cast<HookEvent>(0));
	static_assert(HookEvent::Call == static_cast<HookEvent>(LUA_MASKCALL));
	static_assert(HookEvent::Return == static_cast<HookEvent>(LUA_MASKRET));
	static_assert(HookEvent::Line == static_cast<HookEvent>(LUA_MASKLINE));
	static_assert(HookEvent::Count == static_cast<HookEvent>(LUA_MASKCOUNT));
	static_assert(ComparisonOperator::Equals == static_cast<ComparisonOperator>(LUA_OPEQ));
	static_assert(ComparisonOperator::LessThan == static_cast<ComparisonOperator>(LUA_OPLT));
	static_assert(ComparisonOperator::LessThanOrEquals == static_cast<ComparisonOperator>(LUA_OPLE));
	static_assert(State::REGISTRY_MAINTHREAD == LUA_RIDX_MAINTHREAD);
	static_assert(State::REGISTRY_GLOBALS == LUA_RIDX_GLOBALS);
	static_assert(State::REGISTRY_LOADED_TABLE == LUA_LOADED_TABLE);
	static_assert(State::REGISTRY_PRELOADED_TABLE == LUA_PRELOAD_TABLE);
	static_assert(ArihmeticOperator::Add == static_cast<ArihmeticOperator>(LUA_OPADD));
	static_assert(ArihmeticOperator::Subtract == static_cast<ArihmeticOperator>(LUA_OPSUB));
	static_assert(ArihmeticOperator::Multiply == static_cast<ArihmeticOperator>(LUA_OPMUL));
	static_assert(ArihmeticOperator::Modulo == static_cast<ArihmeticOperator>(LUA_OPMOD));
	static_assert(ArihmeticOperator::Pow == static_cast<ArihmeticOperator>(LUA_OPPOW));
	static_assert(ArihmeticOperator::Divide == static_cast<ArihmeticOperator>(LUA_OPDIV));
	static_assert(ArihmeticOperator::IntegerDivide == static_cast<ArihmeticOperator>(LUA_OPIDIV));
	static_assert(ArihmeticOperator::BitwiseAnd == static_cast<ArihmeticOperator>(LUA_OPBAND));
	static_assert(ArihmeticOperator::BitwiseOr == static_cast<ArihmeticOperator>(LUA_OPBOR));
	static_assert(ArihmeticOperator::BitwiseXOr == static_cast<ArihmeticOperator>(LUA_OPBXOR));
	static_assert(ArihmeticOperator::ShiftLeft == static_cast<ArihmeticOperator>(LUA_OPSHL));
	static_assert(ArihmeticOperator::ShiftRight == static_cast<ArihmeticOperator>(LUA_OPSHR));
	static_assert(ArihmeticOperator::UnaryNegation == static_cast<ArihmeticOperator>(LUA_OPUNM));
	static_assert(ArihmeticOperator::BitwiseNot == static_cast<ArihmeticOperator>(LUA_OPBNOT));
	static_assert(State::EXTRASPACE == LUA_EXTRASPACE);

	ExConverterT ExceptionConverter = nullptr;

	HookEvent LuaHookToEvent(int ev) {
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKCALL) == HookEvent::Call);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKRET) == HookEvent::Return);
		static_assert(static_cast<HookEvent>(1 << LUA_HOOKTAILCALL) == HookEvent::TailCall);
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
		d.nparams = 0;
		d.isvararg = 0;
		d.istailcall = 0;
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
		trg.NumParameters = src.nparams;
		trg.IsVarArg = src.isvararg;
		trg.IsTailCall = src.istailcall;
		std::memcpy(trg.ShortSrc, src.short_src, DebugInfo::SHORTSRC_SIZE);
		trg.ShortSrc[DebugInfo::SHORTSRC_SIZE - 1] = '\0';
		trg.CallInfo = src.i_ci;
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
		luaL_requiref(L, "_G", luaopen_base, true);
		luaL_requiref(L, LUA_COLIBNAME, luaopen_coroutine, true);
		luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, true);
		luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, true);
		luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, true);
		luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, true);
		if (io) {
			luaL_requiref(L, LUA_LOADLIBNAME, luaopen_package, true);
			luaL_requiref(L, LUA_IOLIBNAME, luaopen_io, true);
			luaL_requiref(L, LUA_OSLIBNAME, luaopen_os, true);
		}
		if (debug) {
			luaL_requiref(L, LUA_DBLIBNAME, luaopen_debug, true);
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
		lua_copy(L, from, to);
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
	bool State::IsInteger(int index)
	{
		return lua_isinteger(L, index);
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
	bool State::RawEqual(int i1, int i2)
	{
		return lua_rawequal(L, i1, i2);
	}
	int State::Compare_Unprotected(lua_State* L)
	{
		int op = static_cast<int>(lua_tointeger(L, 4));
		bool r = lua_compare(L, 1, 2, op);
		*static_cast<bool*>(lua_touserdata(L, 3)) = r;
		return 0;
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
		return State{ l };
	}
	const void* State::ToPointer(int index)
	{
		return lua_topointer(L, index);
	}
	void* State::ToUserdata(int index)
	{
		return lua_touserdata(L, index);
	}
	int State::ObjLen_Unprotected(lua_State* L)
	{
		lua_len(L, 1);
		return 1;
	}
	size_t State::RawLength(int index)
	{
		return lua_rawlen(L, index);
	}
	bool State::NumberToInteger(Number n, Integer& i)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
		return lua_numbertointeger(n, &i);
#pragma clang diagnostic pop
	}
	size_t State::StringToNumber(const char* s)
	{
		return lua_stringtonumber(L, s);
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
		lua_pushinteger(L, i);
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
		int op = static_cast<int>(lua_tointeger(L, -1));
		lua_pop(L, 1);
		lua_arith(L, op);
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
		return static_cast<LType>(lua_getuservalue(L, index));
	}
	void State::SetUserValue(int index)
	{
		lua_setuservalue(L, index);
	}
	ErrorCode State::Load(const char* (LUAPP_CDECL* reader)(lua_State*, void*, size_t*), void* ud, const char* chunkname)
	{
		return static_cast<ErrorCode>(lua_load(L, reader, ud, chunkname, nullptr));
	}
	void State::Dump(int(LUAPP_CDECL* writer)(lua_State*, const void*, size_t, void*), void* ud)
	{
		lua_dump(L, writer, ud, false);
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
		lua_pushglobaltable(L);
	}
	int State::Next_Unproteced(lua_State* L)
	{
		bool has = lua_next(L, 2);
		*static_cast<bool*>(lua_touserdata(L, 1)) = has;
		return has ? 2 : 0;
	}
	void State::GetEnvironment(int idx)
	{
		if (!lua_isfunction(L, idx) || lua_iscfunction(L, idx)) {
			PushGlobalTable();
			return;
		}
		int l = 1;
		while (const char* n = lua_getupvalue(L, idx, l)) {
			if (n == std::string_view{ "_ENV" })
				return;
			Pop(1);
			++l;
		}
		Push();
	}
	bool State::SetEnvironment(int idx)
	{
		if (!lua_isfunction(L, idx) || lua_iscfunction(L, idx)) {
			Pop(1);
			return false;
		}
		int l = 1;
		while (const char* n = lua_getupvalue(L, idx, l)) {
			if (n == std::string_view{ "_ENV" }) {
				Pop(1);
				lua_setupvalue(L, idx, l);
				return true;
			}
			Pop(1);
			++l;
		}
		Pop(1);
		return false;
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
	    throw std::logic_error{"unreachable"};
	}
	State State::NewThread()
	{
		return State{ lua_newthread(L) };
	}
	ErrorCode State::ResumeThread(int narg)
	{
		if constexpr (TypeChecks) {
			if (Type(-narg - 1) != LType::Thread)
				throw lua::LuaException{ "ResumeThread trying to resume non thread" };
		}
		return static_cast<ErrorCode>(lua_resume(L, nullptr, narg));
	}
	void State::YieldThread(int nret)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(nret);
		}
		lua_yield(L, nret);
	    throw std::logic_error{"unreachable"};
	}
	void State::XMove(State to, int num)
	{
		if constexpr (TypeChecks) {
			CheckStackHasElements(num);
		}
		lua_xmove(L, to.L, num);
	}
	bool State::IsYieldable()
	{
		return lua_isyieldable(L);
	}
	Number State::Version() {
		return *lua_version(nullptr);
	}
	void* State::GetExtraSpace()
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
		return lua_getextraspace(L);
#pragma clang diagnostic pop
	}
	const char* Debug_GetOptionString(DebugInfoOptions opt, bool pushFunc, bool fromStack)
	{
	    static constexpr auto push = static_cast<DebugInfoOptions>(32);
	    static constexpr auto stack = static_cast<DebugInfoOptions>(64);
	    static constexpr auto lut =
            lut_details::MakeLUT<int,
                                 static_cast<int>(DebugInfoOptions::Name | DebugInfoOptions::Source |
                                                  DebugInfoOptions::Line | DebugInfoOptions::Upvalues | DebugInfoOptions::TailCall | push | stack),
                                 lut_details::OptionStringElement{static_cast<int>(stack), '>'},
                                 lut_details::OptionStringElement{static_cast<int>(push), 'f'},
                                 lut_details::OptionStringElement{static_cast<int>(DebugInfoOptions::Name), 'n'},
                                 lut_details::OptionStringElement{static_cast<int>(DebugInfoOptions::Source), 'S'},
                                 lut_details::OptionStringElement{static_cast<int>(DebugInfoOptions::Line), 'l'},
                                 lut_details::OptionStringElement{static_cast<int>(DebugInfoOptions::Upvalues), 'u'},
                                 lut_details::OptionStringElement{static_cast<int>(DebugInfoOptions::TailCall), 't'}>();

	    opt = opt &
            (DebugInfoOptions::Name | DebugInfoOptions::Source | DebugInfoOptions::Line | DebugInfoOptions::Upvalues | DebugInfoOptions::TailCall);
	    if (pushFunc)
	        opt = opt | push;
	    if (fromStack)
	        opt = opt | stack;
	    return lut[static_cast<size_t>(static_cast<int>(opt))].data();
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
		d.i_ci = nullptr;
		CopyDebugInfo(d, r);
		return r;
	}
	bool State::Debug_PushDebugInfoFunc(const DebugInfo& info)
	{
		lua_Debug d;
		if (info.CallInfo == nullptr)
			return false;
		d.i_ci = static_cast<decltype(d.i_ci)>(info.CallInfo);
		return lua_getinfo(L, Debug_GetOptionString(DebugInfoOptions::None, true, false), &d);
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
	const char* State::Debug_GetLocal(const DebugInfo& info, int localnum)
	{
		lua_Debug d;
		if (info.CallInfo == nullptr)
			throw LuaException{ "invalid DebugInfo" };
		d.i_ci = static_cast<decltype(d.i_ci)>(info.CallInfo);
		return lua_getlocal(L, &d, localnum);
	}
	const char* State::Debug_SetLocal(const DebugInfo& info, int localnum)
	{
		lua_Debug d;
		if (info.CallInfo == nullptr)
			throw LuaException{ "invalid DebugInfo" };
		d.i_ci = static_cast<decltype(d.i_ci)>(info.CallInfo);
		return lua_setlocal(L, &d, localnum);
	}
	const char* State::Debug_GetUpvalue(int index, int upnum)
	{
		return lua_getupvalue(L, index, upnum);
	}
	const char* State::Debug_SetUpvalue(int index, int upnum)
	{
		return lua_setupvalue(L, index, upnum);
	}
	const void* State::Debug_UpvalueID(int index, int upnum)
	{
		return lua_upvalueid(L, index, upnum);
	}
	void State::Debug_UpvalueJoin(int funcMod, int upMod, int funcTar, int upTar)
	{
		lua_upvaluejoin(L, funcMod, upMod, funcTar, upTar);
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
	void State::CheckStackHasElements(int n) {
		int t = GetTop();
		if (t < n)
			throw lua::LuaException{ "stack contains not enough elements" };
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
