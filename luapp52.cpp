#include "../pch.h"

#include "luapp52.h"

#ifndef LUA_CPPLINKAGE
extern "C" {
#endif
#include "..\lua52\lua.h"
#include "..\lua52\lauxlib.h"
#include "..\lua52\lualib.h"
#ifndef LUA_CPPLINKAGE
}
#endif

#include <cstdlib>
#include <type_traits>
#include <sstream>

namespace lua52 {
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
	static_assert(std::is_same<Integer, lua_Integer>::value);
	static_assert(std::is_same<CFunction, lua_CFunction>::value);
	static_assert(std::is_same<CHook, lua_Hook>::value);
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
	static_assert(ArihmeticOperator::Add == static_cast<ArihmeticOperator>(LUA_OPADD));
	static_assert(ArihmeticOperator::Subtract == static_cast<ArihmeticOperator>(LUA_OPSUB));
	static_assert(ArihmeticOperator::Multiply == static_cast<ArihmeticOperator>(LUA_OPMUL));
	static_assert(ArihmeticOperator::Divide == static_cast<ArihmeticOperator>(LUA_OPDIV));
	static_assert(ArihmeticOperator::Modulo == static_cast<ArihmeticOperator>(LUA_OPMOD));
	static_assert(ArihmeticOperator::Pow == static_cast<ArihmeticOperator>(LUA_OPPOW));
	static_assert(ArihmeticOperator::UnaryNegation == static_cast<ArihmeticOperator>(LUA_OPUNM));

	std::string(*ExceptionConverter)(std::exception_ptr ex, const char* funcsig) = nullptr;

	HookEvent LuaHookToEvent(int ev) {
		switch (ev) {
		case LUA_HOOKCALL:
			return HookEvent::Call;
		case LUA_HOOKRET:
			return HookEvent::Return;
		case LUA_HOOKTAILCALL:
			return HookEvent::TailCall;
		case LUA_HOOKLINE:
			return HookEvent::Line;
		case LUA_HOOKCOUNT:
			return HookEvent::Count;
		default:
			return HookEvent::None;
		}
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
		memcpy(trg.ShortSrc, src.short_src, DebugInfo::SHORTSRC_SIZE);
		trg.ShortSrc[DebugInfo::SHORTSRC_SIZE - 1] = '\0';
	}

	State::State(lua_State* L)
	{
		static_assert(Reference::REFNIL == LUA_REFNIL);
		static_assert(Reference::NOREF == LUA_NOREF);
		this->L = L;
	}

	State::State(bool io, bool debug)
	{
		L = luaL_newstate();
		luaL_requiref(L, "_G", luaopen_base, true);
		luaL_requiref(L, LUA_COLIBNAME, luaopen_coroutine, true);
		luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, true);
		luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, true);
		luaL_requiref(L, LUA_BITLIBNAME, luaopen_bit32, true);
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

	State State::Create(bool io, bool debug)
	{
		return State(io, debug);
	}

	void State::Close()
	{
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
	bool State::Equal(int i1, int i2)
	{
		return Compare(i1, i2, ComparisonOperator::Equals);
	}
	bool State::RawEqual(int i1, int i2)
	{
		return lua_rawequal(L, i1, i2);
	}
	bool State::LessThan(int i1, int i2)
	{
		return Compare(i1, i2, ComparisonOperator::LessThan);
	}
	int compare_protected(lua_State* L)
	{
		int op = lua_tointeger(L, 4);
		bool r = lua_compare(L, 1, 2, op);
		*static_cast<bool*>(lua_touserdata(L, 3)) = r;
		return 0;
	}
	bool State::Compare(int i1, int i2, ComparisonOperator op)
	{
		bool ret = false;
		if (!IsValidIndex(i1) || !IsValidIndex(i2))
			return false;
		i1 = ToAbsoluteIndex(i1);
		i2 = ToAbsoluteIndex(i2);
		lua_pushcfunction(L, &compare_protected);
		lua_pushvalue(L, i1);
		lua_pushvalue(L, i2);
		lua_pushlightuserdata(L, &ret);
		lua_pushinteger(L, static_cast<int>(op));
		TCall(4, 0);
		return ret;
	}
	bool State::IsNoneOrNil(int idx)
	{
		return lua_isnoneornil(L, idx);
	}
	bool State::ToBoolean(int index)
	{
		return lua_toboolean(L, index);
	}
	Number State::ToNumber(int index, bool throwIfNotNumber)
	{
		int isnum = 0;
		Number n = lua_tonumberx(L, index, &isnum);
		if (throwIfNotNumber && !isnum)
			throw lua::LuaException{ "ToNumber not a number" };
		return n;
	}
	Integer State::ToInteger(int index, bool throwIfNotNumber)
	{
		int isnum = 0;
		Integer n = lua_tointegerx(L, index, &isnum);
		if (throwIfNotNumber && !isnum)
			throw lua::LuaException{ "ToInteger not a number" };
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
	int protected_len(lua_State* L)
	{
		lua_len(L, 1);
		return 1;
	}
	void State::ObjLength(int index)
	{
		index = ToAbsoluteIndex(index);
		lua_pushcfunction(L, &protected_len);
		PushValue(index);
		TCall(1, 1);
	}
	size_t State::RawLength(int index)
	{
		return lua_rawlen(L, index);
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
	int concat_protected(lua_State* L)
	{
		int n = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		lua_concat(L, n);
		return 1;
	}
	void State::Concat(int num)
	{
		lua_pushcfunction(L, &concat_protected);
		lua_insert(L, -num-1);
		lua_pushnumber(L, num);
		TCall(num + 1, 1);
	}
	int arith_protected(lua_State* L)
	{
		int op = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_arith(L, op);
		return 1;
	}
	void State::Arithmetic(ArihmeticOperator op)
	{
		lua_pushcfunction(L, &arith_protected);
		lua_insert(L, op == ArihmeticOperator::UnaryNegation ? -2 : -3);
		lua_pushinteger(L, static_cast<int>(op));
		TCall(op == ArihmeticOperator::UnaryNegation ? 2 : 3, 1);
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
		lua_getuservalue(L, index);
		return Type(-1);
	}
	void State::SetUserValue(int index)
	{
		lua_setuservalue(L, index);
	}
	ErrorCode State::Load(const char* (__cdecl* reader)(lua_State*, void*, size_t*), void* ud, const char* chunkname)
	{
		return static_cast<ErrorCode>(lua_load(L, reader, ud, chunkname, nullptr));
	}
	void State::Dump(int(__cdecl* writer)(lua_State*, const void*, size_t, void*), void* ud)
	{
		lua_dump(L, writer, ud);
	}
	std::string State::Dump()
	{
		std::stringstream str{};
		Dump([](lua_State* L, const void* data, size_t s, void* ud) {
			auto* st = static_cast<std::stringstream*>(ud);
			st->write(static_cast<const char*>(data), s);
			return 0;
			}, &str);
		return str.str();
	}
	void State::NewTable()
	{
		lua_newtable(L);
	}
	int gettable_protected(lua_State* L)
	{
		lua_gettable(L, 1);
		return 1;
	}
	void State::GetTable(int index)
	{
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		lua_pushcfunction(L, &gettable_protected);
		lua_insert(L, -3);
		TCall(2, 1);
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
	int settable_protected(lua_State* L)
	{
		lua_settable(L, 1);
		return 0;
	}
	void State::SetTable(int index)
	{
		lua_pushvalue(L, index);
		lua_insert(L, -3);
		lua_pushcfunction(L, &settable_protected);
		lua_insert(L, -4);
		TCall(3, 0);
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
	void State::SetGlobal()
	{
		PushGlobalTable();
		Insert(-3);
		SetTableRaw(-3);
		Pop(1);
	}
	void State::SetGlobal(const char* k)
	{
		Push(k);
		Insert(-2);
		SetGlobal();
	}
	void State::GetGlobal()
	{
		PushGlobalTable();
		Insert(-2);
		GetTableRaw(-2);
		Remove(-2);
	}
	void State::GetGlobal(const char* k)
	{
		Push(k);
		GetGlobal();
	}
	void State::PushGlobalTable()
	{
		lua_pushglobaltable(L);
	}
	int next_protected(lua_State* L)
	{
		bool has = lua_next(L, 2);
		*static_cast<bool*>(lua_touserdata(L, 1)) = has;
		return has ? 2 : 0;
	}
	bool State::Next(int index)
	{
		bool r = false;
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		lua_pushlightuserdata(L, &r);
		lua_insert(L, -3);
		lua_pushcfunction(L, &next_protected);
		lua_insert(L, -4);
		TCall(3, MULTIRET);
		return r;
	}
	PairsHolder State::Pairs(int index)
	{
		return PairsHolder(*this, index);
	}
	IPairsHolder State::IPairs(int index)
	{
		return IPairsHolder(*this, index);
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
	void State::TCall(int nargs, int nresults)
	{
		Push<DefaultErrorDecorator>();
		int ehsi = ToAbsoluteIndex(-nargs - 2); // just under the func to be called
		Insert(ehsi);
		ErrorCode c = PCall(nargs, nresults, ehsi);
		if (c != ErrorCode::Success) {
			std::string msg = ErrorCodeFormat(c);
			msg += ToString(-1);
			Pop(1); // error msg
			Remove(ehsi); // DefaultErrorDecorator
			throw LuaException{ msg };
		}
		Remove(ehsi); // DefaultErrorDecorator
	}
	std::string State::int2Str(int i) {
		//return std::format("{0:X}", i);
		return std::to_string(i);
	}
	std::string State::ToDebugString(int index)
	{
		LType t = Type(index);
		switch (t)
		{
		case LType::Nil:
			return "nil";
		case LType::Boolean:
			return ToBoolean(index) ? "true" : "false";
		case LType::LightUserdata:
			return "<LightUserdata " + int2Str(reinterpret_cast<int>(ToUserdata(index))) + ">";
		case LType::Number:
			return std::to_string(ToNumber(index));
		case LType::String:
			return "\"" + ToStdString(index) + "\"";
		case LType::Table:
			return "<table " + int2Str(reinterpret_cast<int>(lua_topointer(L, index))) + ">";
		case LType::Function:
			{
				PushValue(index);
				DebugInfo d = Debug_GetInfoForFunc(DebugInfoOptions::Name | DebugInfoOptions::Source | DebugInfoOptions::Line);
				std::ostringstream name{};
				name << "<function ";
				name << d.What << " ";
				name << d.NameWhat << " ";
				name << (d.Name ? d.Name : "null") << " (defined in:";
				name << d.ShortSrc << ":";
				name << d.CurrentLine << ")>";
				return name.str();
			}
		case LType::Userdata:
			{
				std::string ud = "";
				if (GetMetaField(-1, TypeNameName)) {
					ud = ToString(-1);
					Pop(1);
				}
				return "<Userdata " + ud + + " " + int2Str(reinterpret_cast<int>(ToUserdata(index))) + ">";
			}
		case LType::Thread:
			return "<thread " + int2Str(reinterpret_cast<int>(lua_tothread(L, index))) + ">";
		case LType::None:
			return "<none>";
		default:
			return "<unknown>";
		}
	}
	std::string State::GenerateStackTrace(int levelStart, int levelEnd, bool upvalues, bool locals)
	{
		int lvl = levelStart;
		lua_Debug ar;
		std::ostringstream trace{};
		while (levelEnd != lvl && lua_getstack(L, lvl, &ar)) {
			if (lua_getinfo(L, "nSl", &ar)) {
				trace << "\t";
				trace << ar.what << " ";
				trace << ar.namewhat << " ";
				trace << (ar.name ? ar.name : "null") << " (defined in:";
				trace << ar.short_src << ":";
				trace << ar.currentline << ")";
				if (locals) {
					const char* localname;
					int lnum = 1;
					while (localname = lua_getlocal(L, &ar, lnum)) {
						trace << "\r\n\t\tlocal " << localname << " = " << ToDebugString(-1);
						Pop(1);
						lnum++;
					}
				}
				if (upvalues) {
					lua_getinfo(L, "f", &ar);
					const char* upname;
					int unum = 1;
					while (upname = lua_getupvalue(L, -1, unum)) {
						trace << "\r\n\t\tupvalue " << upname << " = " << ToDebugString(-1);
						Pop(1);
						unum++;
					}
					Pop(1);
				}
				trace << "\r\n";
			}
			lvl++;
		}
		return trace.str();
	}
	int State::DefaultErrorDecorator(State L)
	{
		std::ostringstream trace{};
		trace << L.ToString(-1);
		L.Pop(1);
		trace << "\r\nStacktrace:\r\n";
		trace << L.GenerateStackTrace(1, -1, true, true);
		L.Push(trace.str());
		return 1;
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
	void State::ProtectedAPI(APIProtector* p)
	{
		if (!CheckStack(3))
			throw LuaException("ProtectedAPI: Stack Overflow!");
		Push<ProtectedAPIExecutor>();
		PushLightUserdata(p);
		TCall(1, 0);
	}
	int State::ProtectedAPIExecutor(State L)
	{
		APIProtector* p = static_cast<APIProtector*>(L.ToUserdata(-1));
		p->Work(L);
		return 0;
	}
	void State::RegisterFunc(const char* name, CFunction f, int index)
	{
		Push(name);
		Push(f);
		SetTableRaw(index);
	}
	void State::RegisterFunc(const char* name, CFunction f)
	{
		Push(f);
		SetGlobal(name);
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
		return static_cast<ErrorCode>(lua_resume(L, nullptr, narg));
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
		return *lua_version(nullptr);
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
	HookEvent State::Debug_GetEventFromAR(ActivationRecord ar)
	{
		return LuaHookToEvent(ar.ar->event);
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
	void State::ArgError(int arg, const char* msg)
	{
		if constexpr (CatchExceptions) {
			lua_Debug ar;
			lua_getstack(L, 0, &ar);
			lua_getinfo(L, "n", &ar);
			if (strcmp(ar.namewhat, "method") == 0) {
				arg--;  /* do not count `self' */
				if (arg == 0)  /* error is in the self argument itself? */
					ThrowLuaFormatted("calling `%s' on bad self (%s)", ar.name, msg);
			}
			if (ar.name == NULL)
				ar.name = "?";
			ThrowLuaFormatted("bad argument #%d to `%s' (%s)", arg, ar.name, msg);
		}
		else {
			luaL_argerror(L, arg, msg);
		}
	}
	void State::ArgCheck(bool b, int arg, const char* msg)
	{
		if (!b)
			ArgError(arg, msg);
	}
	bool State::CallMeta(int obj, const char* ev)
	{
		obj = ToAbsoluteIndex(obj);
		if (!GetMetaField(obj, ev)) {
			return false;
		}
		PushValue(obj);
		TCall(1, 1);
		return true;
	}
	bool State::CallMeta(int obj, MetaEvent ev)
	{
		return CallMeta(obj, GetMetaEventName(ev));
	}
	void State::CheckStackHasElements(int n)
	{
		int t = GetTop();
		if (t < n)
			throw lua::LuaException{ "not enough stack elements" };
	}
	void State::CheckAny(int idx)
	{
		if (Type(idx) == LType::None)
			ArgError(idx, "value expected");
	}
	Integer State::CheckInt(int idx)
	{
		if constexpr (CatchExceptions) {
			int isnum = 0;
			Integer n = lua_tointegerx(L, idx, &isnum);
			if (!isnum)
				TypeError(idx, LType::Number);
			return n;
		}
		else {
			return luaL_checkinteger(L, idx);
		}
	}
	const char* State::CheckString(int idx, size_t* len)
	{
		if constexpr (CatchExceptions) {
			const char* s = ToString(idx, len);
			if (!s)
				TypeError(idx, LType::String);
			return s;
		}
		else {
			return luaL_checklstring(L, idx, len);
		}
	}
	std::string State::CheckStdString(int idx)
	{
		size_t l;
		const char* s = CheckString(idx, &l);
		return { s, l };
	}
	std::string State::OptStdString(int idx, const std::string& def)
	{
		size_t l;
		const char* s = OptString(idx, def.c_str(), &l);
		return { s, l };
	}
	std::string_view State::ToStringView(int idx)
	{
		size_t l = 0;
		const char* s = lua_tolstring(L, idx, &l);
		if (!s)
			throw lua::LuaException("no string");
		return { s, l };
	}
	std::string_view State::CheckStringView(int idx)
	{
		size_t l;
		const char* s = CheckString(idx, &l);
		return { s, l };
	}
	std::string_view State::OptStringView(int idx, std::string_view def)
	{
		size_t l;
		const char* s = OptString(idx, def.data(), &l);
		return { s, l };
	}
	const char* State::ConvertToString(int idx, size_t* len)
	{
		idx = ToAbsoluteIndex(idx);
		if (CallMeta(idx, MetaEvent::ToString)) {  /* metafield? */
			if (!IsString(-1))
				throw lua::LuaException{ "'__tostring' must return a string" };
		}
		else {
			switch (Type(idx)) {
			case lua::LType::Number: {
				PushFString("%f", lua_tonumber(L, idx));
				break;
			}
			case lua::LType::String:
				PushValue(idx);
				break;
			case lua::LType::Boolean:
				Push((ToBoolean(idx) ? "true" : "false"));
				break;
			case lua::LType::Nil:
				Push("nil");
				break;
			default: {
				if (GetMetaField(idx, MetaEvent::Name))  /* try name */
				{
					if (IsString(-1)) {
						PushFString("%s: %p", ToString(-1), ToPointer(idx));
						Remove(-2);
						break;
					}
					Remove(-2);
				}
				PushFString("%s: %p", TypeName(Type(idx)), ToPointer(idx));
				break;
			}
			}
		}
		return ToString(-1, len);
	}
	std::string State::ConvertToStdString(int idx)
	{
		size_t len;
		const char* s = ConvertToString(idx, &len);
		return { s, len };
	}
	ErrorCode State::DoString(const std::string& code, const char* name)
	{
		return DoString(code.c_str(), code.length(), name);
	}
	std::string State::LuaVFormat(const char* s, va_list args)
	{
		const char* r = PushVFString(s, args);
		std::string sr = ToStdString(-1);
		Pop(1);
		return sr;
	}
	std::string State::LuaFormat(const char* s, ...)
	{
		va_list args;
		va_start(args, s);
		std::string sr = LuaVFormat(s, args);
		va_end(args);
		return sr;
	}
	void State::ThrowLuaFormatted(const char* s, ...)
	{
		va_list args;
		va_start(args, s);
		std::string sr = LuaVFormat(s, args);
		va_end(args);
		throw LuaException{ sr };
	}
	Number State::CheckNumber(int idx)
	{
		if constexpr (CatchExceptions) {
			int isnum = 0;
			Number n = lua_tonumberx(L, idx, &isnum);
			if (!isnum)
				TypeError(idx, LType::Number);
			return n;
		}
		else {
			return luaL_checknumber(L, idx);
		}
	}
	float State::CheckFloat(int idx)
	{
		return static_cast<float>(CheckNumber(idx));
	}
	bool State::CheckBool(int idx)
	{
		CheckType(idx, LType::Boolean);
		return ToBoolean(idx);
	}
	void State::CheckStack(int extra, const char* msg)
	{
		if constexpr (CatchExceptions) {
			if (!CheckStack(extra))
				ThrowLuaFormatted("stack overflow (%s)", msg);
		}
		else {
			luaL_checkstack(L, extra, msg);
		}
	}
	void State::CheckType(int idx, LType t)
	{
		if (Type(idx) != t)
			TypeError(idx, t);
	}
	void* State::CheckUserdata(int idx, const char* name)
	{
		return luaL_checkudata(L, idx, name);
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
	void State::DoStringT(const char* code, size_t len, const char* name)
	{
		if (!name)
			name = code;
		if (len == 0)
			len = strlen(code);
		ErrorCode e = static_cast<ErrorCode>(luaL_loadbuffer(L, code, len, name));
		if (e != ErrorCode::Success) {
			std::string msg = ErrorCodeFormat(e);
			msg += ToString(-1);
			Pop(1); // error msg
			throw LuaException{ msg };
		}
		TCall(0, MULTIRET);
	}
	void State::Error(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		const char* r = PushVFString(fmt, args);
		va_end(args);
		Error();
	}
	void State::TypeError(int idx, LType t)
	{
		TypeError(idx, TypeName(t));
	}
	void State::TypeError(int idx, const char* t)
	{
		std::string s = LuaFormat("%s expected, got %s", t, TypeName(Type(idx)));
		ArgError(idx, s.c_str());
	}
	void State::Assert(bool a, const char* msg)
	{
		if constexpr (CatchExceptions) {
			if (!a)
				throw LuaException{ msg };
		}
		else {
			if (!a)
				Error(msg);
		}
	}
	bool State::GetMetaField(int obj, const char* ev)
	{
		return luaL_getmetafield(L, obj, ev);
	}
	void State::Where(int lvl)
	{
		luaL_where(L, lvl);
	}
	bool State::GetMetaField(int obj, MetaEvent ev)
	{
		return GetMetaField(obj, GetMetaEventName(ev));
	}
	void State::GetMetaTableFromRegistry(const char* name)
	{
		luaL_getmetatable(L, name);
	}
	bool State::NewMetaTable(const char* name)
	{
		return luaL_newmetatable(L, name);
	}
	bool State::GetSubTable(const char* name, int index)
	{
		index = ToAbsoluteIndex(index);
		Push(name);
		GetTableRaw(index);
		if (!IsTable(-1)) {
			Pop(1);
			NewTable();
			Push(name);
			PushValue(-2);
			SetTableRaw(index);
			return false;
		}
		return true;
	}
	bool State::GetSubTable(const char* name)
	{
		Push(name);
		GetGlobal();
		if (!IsTable(-1)) {
			Pop(1);
			NewTable();
			Push(name);
			PushValue(-2);
			SetGlobal();
			return false;
		}
		return true;
	}
	Integer State::OptInteger(int idx, Integer def)
	{
		if (IsNoneOrNil(idx))
			return def;
		else
			return CheckInt(idx);
	}
	const char* State::OptString(int idx, const char* def, size_t* l)
	{
		if (IsNoneOrNil(idx)) {
			if (l)
				*l = (def ? strlen(def) : 0);
			return def;
		}
		else
			return CheckString(idx, l);
	}
	Number State::OptNumber(int idx, Number def)
	{
		if (IsNoneOrNil(idx))
			return def;
		else
			return CheckNumber(idx);
	}
	bool State::OptBool(int idx, bool def)
	{
		if (IsNoneOrNil(idx))
			return def;
		else
			return ToBoolean(idx);
	}
	float State::OptFloat(int idx, float def)
	{
		return static_cast<float>(OptNumber(idx, def));
	}
	Reference State::Ref(int t)
	{
		return { luaL_ref(L, t) };
	}
	void State::UnRef(Reference r, int t)
	{
		luaL_unref(L, t, r.r);
	}
	void State::Push(Reference r, int t)
	{
		GetTableRaw(t, r.r);
	}
	void State::Push(std::string_view s)
	{
		lua_pushlstring(L, s.data(), s.size());
	}
	std::string State::ToStdString(int idx)
	{
		size_t l = 0;
		const char* s = lua_tolstring(L, idx, &l);
		if (!s)
			throw lua::LuaException("no string");
		return { s, l };
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
	LuaException::LuaException(const std::string& what) : std::runtime_error(what)
	{
	}
	LuaException::LuaException(const char* what) : std::runtime_error(what)
	{
	}
	LuaException::LuaException(const LuaException& other) noexcept : std::runtime_error(other)
	{
	}
	ActivationRecord::ActivationRecord(lua_Debug* ar)
	{
		this->ar = ar;
	}
	StateCloser::StateCloser(State l)
	{
		L = l;
	}
	StateCloser::StateCloser(bool io, bool debug) : L{io, debug}
	{
	}
	StateCloser::~StateCloser()
	{
		L.Close();
	}
	State StateCloser::GetState()
	{
		return L;
	}
	PairsHolder::PairsHolder(State l, int i)
	{
		L = l;
		index = L.ToAbsoluteIndex(i);
	}
	PairsIter PairsHolder::begin()
	{
		L.Push();
		PairsIter i{ L, index };
		i.hasNext = L.Next(index);
		return i;
	}
	PairsSentinel PairsHolder::end()
	{
		return PairsSentinel();
	}
	PairsIter::PairsIter(State l, int i)
	{
		L = l;
		index = i;
	}
	PairsIter& PairsIter::operator++()
	{
		L.Pop(1); // value
		hasNext = L.Next(index);
		return *this;
	}
	PairsIter PairsIter::operator++(int)
	{
		PairsIter r = *this;
		++(*this);
		return r;
	}
	LType PairsIter::operator*()
	{
		return L.Type(-2);
	}
	bool operator==(const PairsIter& i, PairsSentinel s)
	{
		return !i.hasNext;
	}
	bool operator==(PairsSentinel s, const PairsIter& i)
	{
		return i == s;
	}
	bool operator==(const IPairsIter& i, PairsSentinel s)
	{
		return !i.hasNext;
	}
	bool operator==(PairsSentinel s, const IPairsIter& i)
	{
		return i == s;
	}
	IPairsHolder::IPairsHolder(State l, int i)
	{
		L = l;
		index = L.ToAbsoluteIndex(i);
	}
	IPairsIter IPairsHolder::begin()
	{
		IPairsIter i{ L, index };
		L.GetTableRaw(index, i.key);
		if (L.Type(-1) == LType::Nil) {
			i.hasNext = false;
			L.Pop(1);
		}
		else {
			i.hasNext = true;
		}
		return i;
	}
	PairsSentinel IPairsHolder::end()
	{
		return PairsSentinel();
	}
	IPairsIter::IPairsIter(State l, int i)
	{
		L = l;
		index = i;
	}
	IPairsIter& IPairsIter::operator++()
	{
		L.Pop(1);
		++key;
		L.GetTableRaw(index, key);
		if (L.Type(-1) == LType::Nil) {
			hasNext = false;
			L.Pop(1);
		}
		else {
			hasNext = true;
		}
		return *this;
	}
	IPairsIter IPairsIter::operator++(int)
	{
		IPairsIter r = *this;
		++(*this);
		return r;
	}
	int IPairsIter::operator*()
	{
		return key;
	}
};
