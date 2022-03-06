#include "../pch.h"

#include "luapp50.h"

extern "C" {
#include "..\lua50\lua.h"
#include "..\lua50\lauxlib.h"
#include "..\lua50\lualib.h"
}

#include <cstdlib>
#include <type_traits>
#include <sstream>

namespace lua50 {
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
	static_assert(State::GLOBALSINDEX == LUA_GLOBALSINDEX);
	static_assert(State::MULTIRET == LUA_MULTRET);
	static_assert(ErrorCode::Success == static_cast<ErrorCode>(0));
	static_assert(ErrorCode::Runtime == static_cast<ErrorCode>(LUA_ERRRUN));
	static_assert(ErrorCode::Memory == static_cast<ErrorCode>(LUA_ERRMEM));
	static_assert(ErrorCode::ErrorHandler == static_cast<ErrorCode>(LUA_ERRERR));
	static_assert(ErrorCode::Syntax == static_cast<ErrorCode>(LUA_ERRSYNTAX));
	static_assert(ErrorCode::File == static_cast<ErrorCode>(LUA_ERRFILE));
	static_assert(State::Upvalueindex(1) == lua_upvalueindex(1));
	static_assert(State::Upvalueindex(500) == lua_upvalueindex(500));
	static_assert(State::REGISTRYINDEX == LUA_REGISTRYINDEX);
	static_assert(DebugInfo::SHORTSRC_SIZE == LUA_IDSIZE);
	static_assert(HookEvent::None == static_cast<HookEvent>(0));
	static_assert(HookEvent::Call == static_cast<HookEvent>(LUA_MASKCALL));
	static_assert(HookEvent::Return == static_cast<HookEvent>(LUA_MASKRET));
	static_assert(HookEvent::Line == static_cast<HookEvent>(LUA_MASKLINE));
	static_assert(HookEvent::Count == static_cast<HookEvent>(LUA_MASKCOUNT));


	HookEvent LuaHookToEvent(int ev) {
		switch (ev) {
		case LUA_HOOKCALL:
			return HookEvent::Call;
		case LUA_HOOKRET:
			return HookEvent::Return;
		case LUA_HOOKTAILRET:
			return HookEvent::TailReturn;
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
		memcpy(trg.ShortSrc, src.short_src, DebugInfo::SHORTSRC_SIZE);
		trg.ShortSrc[DebugInfo::SHORTSRC_SIZE - 1] = '\0';
	}

	State::State(lua_State* L)
	{
		static_assert(Reference::REFNIL == LUA_REFNIL);
		static_assert(Reference::NOREF == LUA_NOREF);
		this->L = L;
	}

	lua_State* State::GetState()
	{
		return L;
	}

	State State::Create(bool io, bool debug)
	{
		lua_State* L = lua_open();
		luaopen_base(L);
		luaopen_string(L);
		luaopen_table(L);
		luaopen_math(L);
		if (io)
			luaopen_io(L);
		if (debug)
			luaopen_debug(L);
		lua_settop(L, 0);
		return { L };
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
		return lua_equal(L, i1, i2);
	}
	bool State::RawEqual(int i1, int i2)
	{
		return lua_rawequal(L, i1, i2);
	}
	bool State::LessThan(int i1, int i2)
	{
		return lua_lessthan(L, i1, i2);
	}
	bool State::IsNoneOrNil(int idx)
	{
		return lua_isnoneornil(L, idx);
	}
	bool State::ToBoolean(int index)
	{
		return lua_toboolean(L, index);
	}
	Number State::ToNumber(int index)
	{
		return lua_tonumber(L, index);
	}
	Integer State::ToInteger(int index)
	{
		return static_cast<Integer>(lua_tonumber(L, index));
	}
	const char* State::ToString(int index)
	{
		return lua_tostring(L, index);
	}
	size_t State::StringLength(int index)
	{
		return lua_strlen(L, index);
	}
	CFunction State::ToCFunction(int index)
	{
		return lua_tocfunction(L, index);
	}
	State State::ToThread(int index)
	{
		return { lua_tothread(L, index) };
	}
	const void* State::ToPointer(int index)
	{
		return lua_topointer(L, index);
	}
	void* State::ToUserdata(int index)
	{
		return lua_touserdata(L, index);
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
	void State::Concat(int num)
	{
		lua_concat(L, num);
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
	void State::NewTable()
	{
		lua_newtable(L);
	}
	void State::GetTable(int index)
	{
		lua_gettable(L, index);
	}
	void State::GetTableRaw(int index)
	{
		lua_rawget(L, index);
	}
	void State::GetTableRawI(int index, int n)
	{
		lua_rawgeti(L, index, n);
	}
	void State::SetTable(int index)
	{
		lua_settable(L, index);
	}
	void State::SetTableRaw(int index)
	{
		lua_rawset(L, index);
	}
	void State::SetTableRawI(int index, int n)
	{
		lua_rawseti(L, index, n);
	}
	bool State::Next(int index)
	{
		return lua_next(L, index);
	}
	void State::Call(int nargs, int nresults)
	{
		lua_call(L, nargs, nresults);
	}
	ErrorCode State::PCall(int nargs, int nresults, int errfunc)
	{
		return static_cast<ErrorCode>(lua_pcall(L, nargs, nresults, errfunc));
	}
	void State::TCall(int nargs, int nresults)
	{
		Push<DefaultErrorDecorator>();
		Insert(1);
		ErrorCode c = PCall(nargs, nresults, 1);
		if (c != ErrorCode::Success) {
			std::string msg = ErrorCodeFormat(c);
			msg += ToString(-1);
			Pop(1); // error msg
			Remove(1); // DefaultErrorDecorator
			throw LuaException{ msg };
		}
		Remove(1); // DefaultErrorDecorator
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
		case lua50::LType::Nil:
			return "nil";
		case lua50::LType::Boolean:
			return ToBoolean(index) ? "true" : "false";
		case lua50::LType::LightUserdata:
			return "<LightUserdata " + int2Str(reinterpret_cast<int>(ToUserdata(index))) + ">";
		case lua50::LType::Number:
			return std::to_string(ToNumber(index));
		case lua50::LType::String:
			return "\"" + ToStdString(index) + "\"";
		case lua50::LType::Table:
			return "<table " + int2Str(reinterpret_cast<int>(lua_topointer(L, index))) + ">";
		case lua50::LType::Function:
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
		case lua50::LType::Userdata:
			{
				std::string ud = "";
				if (GetMetaField(-1, TypeNameName)) {
					ud = ToString(-1);
					Pop(1);
				}
				return "<Userdata " + ud + + " " + int2Str(reinterpret_cast<int>(ToUserdata(index))) + ">";
			}
		case lua50::LType::Thread:
			return "<thread " + int2Str(reinterpret_cast<int>(lua_topointer(L, index))) + ">";
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
		L.Push(trace.str().c_str());
		return 1;
	}
	const char* State::ErrorCodeFormat(ErrorCode c)
	{
		switch (c)
		{
		case lua50::ErrorCode::Success:
			return "Lua_Success: ";
		case lua50::ErrorCode::Runtime:
			return "Lua_RuntimeError: ";
		case lua50::ErrorCode::File:
			return "Lua_FileError: ";
		case lua50::ErrorCode::Syntax:
			return "Lua_SyntaxError: ";
		case lua50::ErrorCode::Memory:
			return "Lua_MemoryError: ";
		case lua50::ErrorCode::ErrorHandler:
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
		return static_cast<ErrorCode>(lua_resume(L, narg));
	}
	int State::YieldThread(int nret)
	{
		return lua_yield(L, nret);
	}
	void State::XMove(State to, int num)
	{
		lua_xmove(L, to.L, num);
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
		return luaL_callmeta(L, obj, ev);
	}
	bool State::CallMeta(int obj, MetaEvent ev)
	{
		return CallMeta(obj, GetMetaEventName(ev));
	}
	void State::CheckAny(int idx)
	{
		if (Type(idx) == LType::None)
			ArgError(idx, "value expected");
	}
	Integer State::CheckInt(int idx)
	{
		return static_cast<Integer>(CheckNumber(idx));
	}
	const char* State::CheckString(int idx, size_t* len)
	{
		if constexpr (CatchExceptions) {
			const char* s = ToString(idx);
			if (!s)
				TypeError(idx, LType::String);
			if (len)
				*len = StringLength(idx);
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
	void State::Check(int idx, Integer* i)
	{
		*i = CheckInt(idx);
	}
	void State::Check(int idx, std::string* s)
	{
		*s = CheckStdString(idx);
	}
	void State::Check(int idx, Number* n)
	{
		*n = CheckNumber(idx);
	}
	Number State::CheckNumber(int idx)
	{
		if constexpr (CatchExceptions) {
			Number n = ToNumber(idx);
			if (n == 0 && !IsNumber(idx))
				TypeError(idx, LType::Number);
			return n;
		}
		else {
			return luaL_checknumber(L, idx);
		}
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
		return static_cast<ErrorCode>(lua_dofile(L, filename));
	}
	ErrorCode State::DoString(const char* code)
	{
		return static_cast<ErrorCode>(lua_dostring(L, code));
	}
	ErrorCode State::DoString(const char* code, size_t l, const char* name)
	{
		return static_cast<ErrorCode>(lua_dobuffer(L, code, l, name));
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
		if constexpr (CatchExceptions) {
			std::string s = LuaFormat("%s expected, got %s", t, TypeName(Type(idx)));
			ArgError(idx, s.c_str());
		}
		else {
			luaL_typerror(L, idx, t);
		}
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
		GetTableRawI(t, r.r);
	}
	void State::Push(const std::string& s)
	{
		lua_pushlstring(L, s.c_str(), s.size());
	}
	std::string State::ToStdString(int idx)
	{
		const char* s = lua_tostring(L, idx);
		size_t l = lua_strlen(L, idx);
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
};
