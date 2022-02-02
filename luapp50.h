#pragma once
#include <cstdarg>
#include <string>
#include <compare>
#include <exception>
#include <stdexcept>
#include <functional>
//#include <format>
#include "constexprTypename.h"

// create a lua::State from this to interact with it
struct lua_State;


namespace lua50 {
	// turn on/off exception handling at compile time
	// if active, CppToCFunction catches any c++ exceptions and converts them to lua exceptions
	// (this gets used internally as well)
	constexpr const bool CatchExceptions = true;

	enum class LType : int {
		Nil,
		Boolean,
		LightUserdata,
		Number,
		String,
		Table,
		Function,
		Userdata,
		Thread,
	};
	enum class ErrorCode : int {
		Success,
		Runtime,
		File,
		Syntax,
		Memory,
		ErrorHandler,
	};
	enum class MetaEvent {
		Add,
		Subtract,
		Multiply,
		Divide,
		Pow,
		UnaryMinus,
		Concat,
		Equals,
		LessThan,
		LessOrEquals,
		Index,
		NewIndex,
		Call,
		Finalizer,
		WeakTable,
	};
	enum class DebugInfoOptions : int {
		None = 0,
		Name = 1,
		Source = 2,
		Line = 4,
		Upvalues = 8,
	};
	struct DebugInfo {
		static constexpr size_t SHORTSRC_SIZE = 60;

		int Event = 0;
		const char* Name = nullptr;
		const char* NameWhat = nullptr;
		const char* What = nullptr;
		const char* Source = nullptr;
		int CurrentLine = 0;
		int NumUpvalues = 0;
		int LineDefined = 0;
		char ShortSrc[SHORTSRC_SIZE] = {};
	};
	constexpr DebugInfoOptions operator|(DebugInfoOptions a, DebugInfoOptions b) {
		using under = std::underlying_type<DebugInfoOptions>::type;
		return static_cast<DebugInfoOptions>(static_cast<under>(a) | static_cast<under>(b));
	}
	constexpr DebugInfoOptions operator&(DebugInfoOptions a, DebugInfoOptions b) {
		using under = std::underlying_type<DebugInfoOptions>::type;
		return static_cast<DebugInfoOptions>(static_cast<under>(a) & static_cast<under>(b));
	}
	constexpr DebugInfoOptions operator^(DebugInfoOptions a, DebugInfoOptions b) {
		using under = std::underlying_type<DebugInfoOptions>::type;
		return static_cast<DebugInfoOptions>(static_cast<under>(a) ^ static_cast<under>(b));
	}

	class State;

	class LuaException : public std::runtime_error {
	public:
		LuaException(const std::string& what);
		LuaException(const char* what);
		LuaException(const LuaException& other) noexcept;
	};

	using Number = double;
	using Integer = int;
	using CFunction = int (*) (lua_State* L);
	using CppFunction = int (*) (State L);

	template<CppFunction F>
	int CppToCFunction(lua_State* l) {
		if constexpr (CatchExceptions) {
			State L{ l };
			try {
				return F(L);
			}
			catch (std::exception& e) {
				L.Error("%s: %s in %s", typeid(e).name(), e.what(), __FUNCSIG__);
			}
			catch (...) {
				L.Error("unnown excetion cathed in %s", __FUNCSIG__);
			}
		}
		else {
			State L{ l };
			return F(L);
		}
	}

	struct FuncReference {
		const char* Name;
		CFunction Func;

		template<CppFunction F>
		constexpr static FuncReference GetRef(const char* name) {
			return { name, &CppToCFunction<F> };
		}
		template<CFunction F>
		constexpr static FuncReference GetRef(const char* name) {
			return { name, F };
		}

		auto operator<=>(const FuncReference&) const = default;
	};

	template<class T>
	concept HasLuaMethods = requires {
		{T::LuaMethods.begin()} -> std::input_iterator;
		{T::LuaMethods.end()} -> std::input_iterator;
	};
	template<class T>
	concept EquatableCpp = std::is_same_v<CppFunction, decltype(&T::Equals)> || std::is_same_v<CFunction, decltype(&T::Equals)>;
	template<class T>
	concept EquatableOp = std::equality_comparable<T>;

	// only a int, so pass by value is preferred
	class Reference {
		friend class State;
		int r;

		constexpr static int NOREF = -2;
		constexpr static int REFNIL = -1;

		constexpr Reference(int r)
		{
			this->r = r;
		}
	public:
		// initialized with noref
		constexpr Reference()
		{
			r = NOREF;
		}

		auto operator<=>(const Reference&) const = default;
	};

	class APIProtector {
	public:
		virtual void Work(State L) = 0;
	};
	template<class T>
	requires std::invocable<T, State>
	class APIProtectorInvoke : public APIProtector {
		T Invoke;
	public:
		APIProtectorInvoke(T i) {
			Invoke = i;
		}
		void Work(State L) {
			std::invoke(Invoke, L);
		}
	};

	// contains only a pointer, so pass-by-value is prefered
	// you need to close this state manually
	class State {
	private:
		lua_State* L;

	public:
		State(lua_State* L);

		static State Create(bool io = true, bool debug = false);
		void Close();

		int GetTop();
		bool CheckStack(int extra);
		constexpr static int MINSTACK = 20;
		bool IsValidIndex(int i);

		void SetTop(int index);
		void PushValue(int index);
		void Remove(int index);
		// inserts tos to index
		void Insert(int index);
		// replaces index with tos
		void Replace(int index);
		void Pop(int num);

		LType Type(int index);
		bool IsNil(int index);
		bool IsBoolean(int index);
		bool IsNumber(int index);
		bool IsString(int index);
		bool IsTable(int index);
		bool IsFunction(int index);
		bool IsCFunction(int index);
		bool IsUserdata(int index);
		bool IsLightUserdata(int index);
		const char* TypeName(LType t);
		bool Equal(int i1, int i2);
		bool RawEqual(int i1, int i2);
		bool LessThan(int i1, int i2);

		bool ToBoolean(int index);
		Number ToNumber(int index);
		Integer ToInteger(int index);
		// warning: converts value on the stack to a string
		const char* ToString(int index);
		size_t StringLength(int index);
		CFunction ToCFunction(int index);
		State ToThread(int index);
		// only useful for debugging
		const void* ToPointer(int index);
		void* ToUserdata(int index);

		void Push(bool b);
		void Push(Number n);
		void Push(Integer i);
		void Push(const char* s);
		void Push(const char* s, size_t l);
		// pushes nil
		void Push();
		void Push(CFunction f, int nups = 0);
		template<CFunction F>
		void Push(int nups = 0)
		{
			Push(F, nups);
		}
		template<CppFunction F>
		void Push(int nups = 0)
		{
			Push(&CppToCFunction<F>, nups);
		}
		void PushLightUserdata(void* ud);
		const char* PushVFString(const char* s, va_list argp);
		// %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char
		const char* PushFString(const char* s, ...);
		void Concat(int num);

		bool GetMetatable(int index);
		// metatable at tos
		bool SetMetatable(int index);

		void* NewUserdata(size_t s);

		void NewTable();
		void GetTable(int index);
		void GetTableRaw(int index);
		void GetTableRawI(int index, int n);
		void SetTable(int index);
		void SetTableRaw(int index);
		void SetTableRawI(int index, int n);
		bool Next(int index);

		constexpr static int GLOBALSINDEX = -10001;
		// no access to fenv

		constexpr static int MULTIRET = -1;
		void Call(int nargs, int nresults);
		ErrorCode PCall(int nargs, int nresults, int errfunc = 0);
		// throws errors as LuaException C++ exception
		void TCall(int nargs, int nresults);
		// attaches stacktrace to TOS string, intended for use with pcall
		static int DefaultErrorDecorator(State L);
		static const char* ErrorCodeFormat(ErrorCode c);
		// allows to use the lua api without running into lua errors that end up in panic. throws errors as LuaException C++ exception
		void ProtectedAPI(APIProtector* p);
	private:
		static int ProtectedAPIExecutor(State L);

	public:
		void RegisterFunc(const char* name, CFunction f, int index = GLOBALSINDEX);
		template<CFunction F>
		void RegisterFunc(const char* name, int index = GLOBALSINDEX) {
			RegisterFunc(name, F, index);
		}
		template<CppFunction F>
		void RegisterFunc(const char* name, int index = GLOBALSINDEX)
		{
			RegisterFunc(name, &CppToCFunction<F>, index);
		}
		template<class T>
		void RegisterFuncs(const T& funcs, int index = GLOBALSINDEX) {
			for (const FuncReference& f : funcs) {
				RegisterFunc(f.Name, f.Func, index);
			}
		}
		template<class T>
		void RegisterGlobalLib(const T& funcs, const char* name) {
			Push(name);
			Push(name);
			GetTableRaw(GLOBALSINDEX);
			if (!IsTable(-1)) {
				Pop(1);
				NewTable();
			}
			RegisterFuncs(funcs, -3);
			SetTableRaw(GLOBALSINDEX);
		}

		constexpr static int Upvalueindex(int i)
		{
			return GLOBALSINDEX - i;
		}
		constexpr static int REGISTRYINDEX = -10000;
		
		[[noreturn]] void Error();

		State NewThread();
		ErrorCode Resume(int narg);
		// warning: this func cannot return (you cannot yield through lua/c boundaries in lua 5.0)
		int Yield(int nret);
		// moves from this to to, warning do only move between coroutines of the same state, g cgets confused otherwise.
		void XMove(State to, int num);

	private:
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
	public:
		bool Debug_GetStack(int level, DebugInfo& Info, DebugInfoOptions opt, bool pushFunc);
		bool Debug_GetInfoForFunc(DebugInfo& Info, DebugInfoOptions opt);
		// debug

		// auxlib (luaL_)
		constexpr const char* GetMetaEventName(MetaEvent f) {
			switch (f)
			{
			case MetaEvent::Add:
				return "__add";
			case MetaEvent::Subtract:
				return "__sub";
			case MetaEvent::Multiply:
				return "__mul";
			case MetaEvent::Divide:
				return "__div";
			case MetaEvent::Pow:
				return "__pow";
			case MetaEvent::UnaryMinus:
				return "__unm";
			case MetaEvent::Concat:
				return "__concat";
			case MetaEvent::Equals:
				return "__eq";
			case MetaEvent::LessThan:
				return "__lt";
			case MetaEvent::LessOrEquals:
				return "__le";
			case MetaEvent::Index:
				return "__index";
			case MetaEvent::NewIndex:
				return "__newindex";
			case MetaEvent::Call:
				return "__call";
			case MetaEvent::Finalizer:
				return "__gc";
			case MetaEvent::WeakTable:
				return "__mode";
			default:
				return "";
			};
		}

		// bad argument #<narg> to <func> (<extramsg>)
		[[noreturn]] void ArgError(int arg, const char* msg);
		void ArgCheck(bool b, int arg, const char* msg);
		bool CallMeta(int obj, const char* ev);
		bool CallMeta(int obj, MetaEvent ev);

		void CheckAny(int idx);
		Integer CheckInt(int idx);
		const char* CheckString(int idx);
		const char* CheckString(int idx, size_t* len = nullptr);
		Number CheckNumber(int idx);
		// checkoption
		// luaerror on fail
		void CheckStack(int extra, const char* msg);
		void CheckType(int idx, LType t);
		void* CheckUserdata(int idx, const char* name);

		ErrorCode DoFile(const char* filename);
		ErrorCode DoString(const char* code);
		ErrorCode DoString(const char* code, size_t l, const char* name);
		ErrorCode LoadBuffer(const char* code, size_t len, const char* name);
		ErrorCode LoadFile(const char* filename);

		[[noreturn]] void Error(const char* fmt, ...);
		[[noreturn]] void TypeError(int idx, LType t);
		void Where(int lvl);

		bool GetMetaField(int obj, const char* ev);
		bool GetMetaField(int obj, MetaEvent ev);
		void GetMetaTable(const char* name);
		bool NewMetaTable(const char* name);

		Integer OptInteger(int idx, Integer def);
		const char* OptString(int idx, const char* def);
		const char* OptString(int idx, const char* def, size_t* l);
		Number OptNumber(int idx, Number def);

		Reference Ref(int t = REGISTRYINDEX);
		void UnRef(Reference r, int t = REGISTRYINDEX);
		void Push(Reference r, int t = REGISTRYINDEX);

		constexpr static Reference NoRef{ Reference::NOREF };
		constexpr static Reference RefNil{ Reference::REFNIL };


		void Push(const std::string& s);
		std::string ToStdString(int idx);
		std::string CheckStdString(int idx);
		std::string OptStdString(int idx, const std::string& def);
		ErrorCode DoString(const std::string& code, const char* name);

	private:
		template<class T>
		static int UserData_Finalizer(State L)
		{
			L.GetUserData<T>(1)->~T();
			return 0;
		}
		template<class T>
		static int UserData_EqualsOperator(State L) {
			if (L.GetTop() < 2) {
				L.Push(false);
				return 1;
			}
			T* t = L.OptionalUserData<T>(1);
			T* o = L.OptionalUserData<T>(2);
			if (t && o) {
				L.Push(*t == *o);
				return 1;
			}
			L.Push(false);
			return 1;
		}

	public:
		template<class T>
		T* OptionalUserData(int i) {
			return static_cast<T*>(CheckUserdata(i, typename_details::type_name<T>()));
		}
		template<class T>
		T* GetUserData(int i) {
			T* t = OptionalUserData<T>(i);
			if (t == nullptr)
				Error("no %s at argument %d", typename_details::type_name<T>(), i);
			return t;
		}
		template<class T>
		void GetUserDataMetatable() {
			if (NewMetaTable(typename_details::type_name<T>())) {
				if constexpr (HasLuaMethods<T>) {
					Push(GetMetaEventName(MetaEvent::Index));
					NewTable();
					RegisterFuncs(T::LuaMethods, -3);
					SetTableRaw(-3);
				}

				if constexpr (!std::is_trivially_destructible_v<T>)
					RegisterFunc(GetMetaEventName(MetaEvent::Finalizer), &CppToCFunction<UserData_Finalizer<T>>, -3);

				if constexpr (EquatableCpp<T>)
					RegisterFunc<T::Equals>(GetMetaEventName(MetaEvent::Equals), -3);
				else if constexpr (EquatableOp<T>)
					RegisterFunc(GetMetaEventName(MetaEvent::Equals), &CppToCFunction<UserData_EqualsOperator<T>>, -3);
			}
		}
		template<class T>
		void PrepareUserDataType() {
			GetUserDataMetatable<T>();
			Pop(1);
		}
		/* converts a c++ class to a lua userdata. creates a new full userdata and calls the constructor of T, forwarding all arguments.
		* a class (metatable) for a userdata type is only generated once, and then reused for all userdata of the same type.
		* 
		* 
		* lua class generation:
		* if T::LuaMethods is iterable over LuaReference, registers them all as userdata methods (__index).
		* 
		* if T is not trivially destructable, generates a finalizer (__gc) that calls its destructor.
		* 
		* if T has a static CFunction or CppFunction Equals, defines a comparator (__eq) with it.
		* else if T has a operator overload for ==, defines a comparator (__eq) with that (checks type, then equality).
		* 
		*/
		template<class T, class ... Args>
		T* NewUserData(Args&& ... args) {
			T* t = new (NewUserdata(sizeof(T))) T(std::forward<Args>(args)...);
			GetUserDataMetatable<T>();
			SetMetatable(-2);
			return t;
		}
	private:
		static std::string int2Str(int i) {
			//return std::format("{0:X}", i);
			return std::to_string(i);
		}
	public:
		template<class T>
		std::string AnalyzeUserDataType() {
			GetUserDataMetatable<T>();
			std::string re{ "analyzing type: " };
			re += typename_details::type_name<T>();

			Push(GetMetaEventName(MetaEvent::Index));
			GetTableRaw(-2);
			if (IsTable(-1)) {
				re += "\nmethod list:";
				Push();
				while (Next(-2)) {
					if (Type(-2) == LType::String) {
						re += "\n\t";
						re += ToString(-2);
						re += " ";
						re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
					}
					Pop(1);
				}
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Finalizer));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nfinalizer ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Equals));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\ncomparator equals ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			re += "\ncomplete";
			Pop(1);
			return re;
		}
	};
}

#ifndef LuaVersion
#define LuaVersion 5.0
namespace lua = lua50;
#endif

