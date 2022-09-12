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
struct lua_Debug;


namespace lua50 {
	/// <summary>
	/// turn on/off exception handling at compile time
	/// if active, CppToCFunction catches any c++ exceptions and converts them to lua exceptions
	/// (this gets used internally as well)
	/// </summary>
	constexpr const bool CatchExceptions = true;

	/// <summary>
	/// all values in lua are of one of these types:
	/// </summary>
	enum class LType : int {
		/// <summary>
		/// represents currently no value (aka nullptr).
		/// </summary>
		Nil,
		/// <summary>
		/// represents a boolean (true/false) value.
		/// </summary>
		Boolean,
		/// <summary>
		/// represents a light userdata (void*) value.
		/// just the pointer, not the actual memory behind it.
		/// </summary>
		LightUserdata,
		/// <summary>
		/// represents a number (double) value.
		/// </summary>
		Number,
		/// <summary>
		/// represents a string (const char*) value.
		/// </summary>
		String,
		/// <summary>
		/// represents a table value. tables are arrays and dictionarys in one.
		/// </summary>
		Table,
		/// <summary>
		/// represents a function (either lua or C/C++) value.
		/// </summary>
		Function,
		/// <summary>
		/// represents a full userdata value.
		/// a block of raw memory C/C++ code can use.
		/// might contain a C++ class.
		/// </summary>
		Userdata,
		/// <summary>
		/// represents a thread (coroutine) value.
		/// </summary>
		Thread,
		/// <summary>
		/// represents a unused stack position.
		/// </summary>
		None = -1,
	};
	/// <summary>
	/// error codes used by lua.
	/// </summary>
	enum class ErrorCode : int {
		/// <summary>
		/// no error.
		/// </summary>
		Success,
		/// <summary>
		/// lua error at runtime.
		/// </summary>
		Runtime,
		/// <summary>
		/// IO error reading or writing files.
		/// </summary>
		File,
		/// <summary>
		/// syntax error parsing lua code.
		/// </summary>
		Syntax,
		/// <summary>
		/// out of memory.
		/// </summary>
		Memory,
		/// <summary>
		/// error processing an error handler.
		/// </summary>
		ErrorHandler,
	};
	/// <summary>
	/// metaevents used in metatables.
	/// </summary>
	enum class MetaEvent {
		/// <summary>
		/// + operator.
		/// </summary>
		Add,
		/// <summary>
		/// - operator.
		/// </summary>
		Subtract,
		/// <summary>
		/// + operator.
		/// </summary>
		Multiply,
		/// <summary>
		/// / operator.
		/// </summary>
		Divide,
		/// <summary>
		/// ^ operator.
		/// </summary>
		Pow,
		/// <summary>
		/// unary - operator.
		/// </summary>
		UnaryMinus,
		/// <summary>
		/// .. operator.
		/// </summary>
		Concat,
		/// <summary>
		/// == operator
		/// </summary>
		Equals,
		/// <summary>
		/// &lt; operator;
		/// </summary>
		LessThan,
		/// <summary>
		/// &lt;= operator;
		/// </summary>
		LessOrEquals,
		/// <summary>
		/// table read operator.
		/// (only when not found in table)
		/// </summary>
		Index,
		/// <summary>
		/// table write operator.
		/// </summary>
		NewIndex,
		/// <summary>
		/// function call operator.
		/// </summary>
		Call,
		/// <summary>
		/// finalizer (has to be present when the metatable is applied).
		/// </summary>
		Finalizer,
		/// <summary>
		/// weak table modes.
		/// </summary>
		WeakTable,
		/// <summary>
		/// function to convert to a string. only used in ConvertToString Luapp methods.
		/// </summary>
		ToString,
		/// <summary>
		/// userdata class name.
		/// </summary>
		Name,
	};
	/// <summary>
	/// options which fields of DebugInfo to fill.
	/// you may binary or the fields to fill multiple.
	/// </summary>
	enum class DebugInfoOptions : int {
		/// <summary>
		/// nothing.
		/// </summary>
		None = 0,
		/// <summary>
		/// Name and NameWhat fields.
		/// </summary>
		Name = 1,
		/// <summary>
		/// What, Source, LineDefined, ShortSrc fields.
		/// </summary>
		Source = 2,
		/// <summary>
		/// CurrentLine field.
		/// </summary>
		Line = 4,
		/// <summary>
		/// NumUpvalues field.
		/// </summary>
		Upvalues = 8,
	};
	/// <summary>
	/// events in DebugInfo::Event and as condition specifier for Debug_SetHook.
	/// </summary>
	enum class HookEvent : int {
		/// <summary>
		/// DebugInfo not from a hook
		/// </summary>
		None = 0,
		/// <summary>
		/// calling a function (before the function gets its arguments)
		/// </summary>
		Call = 1,
		/// <summary>
		/// leaving a function
		/// </summary>
		Return = 2,
		/// <summary>
		/// when executing a new line of code, or jumping back to a line of code (even back to the same)
		/// </summary>
		Line = 4,
		/// <summary>
		/// every count instructions (set via sethook)
		/// </summary>
		Count = 8,
		/// <summary>
		/// leaving a function via a tail return
		/// (meaning lua skipped the stack frame of that function, which makes any calls to Debug_GetInfoFromAR useless)
		/// (requested via Return)
		/// </summary>
		TailReturn = 16,
	};
	/// <summary>
	/// debug info for a function/stack level. see DebugInfoOptions for what to fill.
	/// </summary>
	struct DebugInfo {
		static constexpr size_t SHORTSRC_SIZE = 60;

		HookEvent Event = HookEvent::None;
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
	constexpr HookEvent operator|(HookEvent a, HookEvent b) {
		using under = std::underlying_type<HookEvent>::type;
		return static_cast<HookEvent>(static_cast<under>(a) | static_cast<under>(b));
	}
	constexpr HookEvent operator&(HookEvent a, HookEvent b) {
		using under = std::underlying_type<HookEvent>::type;
		return static_cast<HookEvent>(static_cast<under>(a) & static_cast<under>(b));
	}
	constexpr HookEvent operator^(HookEvent a, HookEvent b) {
		using under = std::underlying_type<HookEvent>::type;
		return static_cast<HookEvent>(static_cast<under>(a) ^ static_cast<under>(b));
	}
	/// <summary>
	/// operators for comparisons.
	/// </summary>
	enum class ComparisonOperator : int {
		/// <summary>
		/// == opeator.
		/// </summary>
		Equals = 0,
		/// <summary>
		/// < operator.
		/// </summary>
		LessThan = 1,
		/// <summary>
		/// <= operator.
		/// </summary>
		LessThanOrEquals = 2,
	};
	/// <summary>
	/// operators for arithmetic operations.
	/// </summary>
	enum class ArihmeticOperator : int {
		/// <summary>
		/// + operator
		/// </summary>
		Add = 0,
		/// <summary>
		/// - operator
		/// </summary>
		Subtract = 1,
		/// <summary>
		/// * operator
		/// </summary>
		Multiply = 2,
		/// <summary>
		/// / operator
		/// </summary>
		Divide = 3,
		/// <summary>
		/// % operator
		/// </summary>
		Modulo = 4,
		/// <summary>
		/// ^ operator
		/// </summary>
		Pow = 5,
		/// <summary>
		/// unary - operator
		/// </summary>
		UnaryNegation = 6,
	};

	class State;

	/// <summary>
	/// exception thrown by lua
	/// </summary>
	class LuaException : public std::runtime_error {
	public:
		LuaException(const std::string& what);
		LuaException(const char* what);
		LuaException(const LuaException& other) noexcept;
	};

	/// <summary>
	/// default number type. any number in lua 5.0 is of this type.
	/// </summary>
	using Number = double;
	/// <summary>
	/// integer type. in lua 5.0 internal representation always as Number, used only as typecasts for convienience functions.
	/// </summary>
	using Integer = int;
	/// <summary>
	/// aka lua_CFunction. no type conversion/exception handling. use CppFunction when in doubt.
	/// </summary>
	/// <see cref='lua50::CppFunction'/>
	/// <see cref='lua50::CppToCFunction'/>
	/// <param name="L">lua state</param>
	/// <returns>number of return values on the stack</returns>
	using CFunction = int(__cdecl*) (lua_State* L);
	/// <summary>
	/// <para>normal function to interface with lua.</para>
	/// <para>recieves its arguments on the lua stack in direct order (first argument at 1) (with nothing else on the stack).</para>
	/// <para>to return, push arguments onto the stack in direct order, and return the number of returns from the func.</para>
	/// </summary>
	/// <param name="L">lua state</param>
	/// <returns>number of return values on the stack</returns>
	using CppFunction = int (*) (State L);

	/// <summary>
	/// adapts a CppFunction to a CFunction (aka lua_CFunction), doing all the type conversion and exception handling.
	/// </summary>
	/// <param name="l">lua state</param>
	/// <returns>number of return values on the stack</returns>
	template<CppFunction F>
	int CppToCFunction(lua_State* l) {
		if constexpr (CatchExceptions) {
			bool err = false;
			State L{ l }; // trivial, no destructor to call, so its save
			int ret = 0;
			{ // make sure all c++ objects gets their destructor called
				try {
					ret = F(L);
				}
				catch (const std::exception& e) {
					L.PushFString("%s: %s in %s", typeid(e).name(), e.what(), __FUNCSIG__);
					err = true;
				}
				catch (...) {
					L.PushFString("unnown excetion catched in %s", __FUNCSIG__);
					err = true;
				}
			}
			if (err)
				L.Error();
			return ret;
		}
		else {
			State L{ l };
			return F(L);
		}
	}

	/// <summary>
	/// info to register a function to lua.
	/// </summary>
	struct FuncReference {
		const char* Name;
		CFunction Func;

		constexpr FuncReference(const char* name, CFunction f) {
			Name = name;
			Func = f;
		}

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

	/// <summary>
	/// checks if a type has methods to register for a userdata type via LuaMethods.
	/// </summary>
	template<class T>
	concept HasLuaMethods = requires {
		{T::LuaMethods.begin()} -> std::input_iterator;
		{T::LuaMethods.end()} -> std::input_iterator;
	};
	/// <summary>
	/// checks if a type has a userdata equals defined manually via Equals static member.
	/// </summary>
	template<class T>
	concept EquatableCpp = std::is_same_v<CppFunction, decltype(&T::Equals)> || std::is_same_v<CFunction, decltype(&T::Equals)>;
	/// <summary>
	/// checks if a type has a userdata equals defined via c++ operator.
	/// </summary>
	template<class T>
	concept EquatableOp = std::equality_comparable<T>;
	/// <summary>
	/// checks if a type has a userdata lessthan defined manually via LessThan static member.
	/// </summary>
	template<class T>
	concept LessThanCpp = std::is_same_v<CppFunction, decltype(&T::LessThan)> || std::is_same_v<CFunction, decltype(&T::LessThan)>;
	/// <summary>
	/// checks if a type has a userdata lessthan defined via c++ operator.
	/// </summary>
	template<class T>
	concept LessThanOp = requires (T a, T b) {
		{a < b} -> std::same_as<bool>;
	};
	/// <summary>
	/// checks if a type has a userdata add defined manually via Add static member.
	/// </summary>
	template<class T>
	concept AddCpp = std::is_same_v<CppFunction, decltype(&T::Add)> || std::is_same_v<CFunction, decltype(&T::Add)>;
	/// <summary>
	/// checks if a type has a userdata add defined via c++ operator.
	/// </summary>
	template<class T>
	concept AddOp = std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a + b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata subtract defined manually via Substract static member.
	/// </summary>
	template<class T>
	concept SubtractCpp = std::is_same_v<CppFunction, decltype(&T::Substract)> || std::is_same_v<CFunction, decltype(&T::Substract)>;
	/// <summary>
	/// checks if a type has a userdata subtract defined via c++ operator.
	/// </summary>
	template<class T>
	concept SubtractOp = std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a - b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata multiply defined manually via Multiply static member.
	/// </summary>
	template<class T>
	concept MultiplyCpp = std::is_same_v<CppFunction, decltype(&T::Multiply)> || std::is_same_v<CFunction, decltype(&T::Multiply)>;
	/// <summary>
	/// checks if a type has a userdata multiply defined via c++ operator.
	/// </summary>
	template<class T>
	concept MultiplyOp = std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a * b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata divide defined manually via Divide static member.
	/// </summary>
	template<class T>
	concept DivideCpp = std::is_same_v<CppFunction, decltype(&T::Divide)> || std::is_same_v<CFunction, decltype(&T::Divide)>;
	/// <summary>
	/// checks if a type has a userdata divide defined via c++ operator.
	/// </summary>
	template<class T>
	concept DivideOp = std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a / b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata pow defined manually via Pow static member.
	/// </summary>
	template<class T>
	concept PowCpp = std::is_same_v<CppFunction, decltype(&T::Pow)> || std::is_same_v<CFunction, decltype(&T::Pow)>;
	/// <summary>
	/// checks if a type has a userdata unary minus defined manually via UnaryMinus static member.
	/// </summary>
	template<class T>
	concept UnaryMinusCpp = std::is_same_v<CppFunction, decltype(&T::UnaryMinus)> || std::is_same_v<CFunction, decltype(&T::UnaryMinus)>;
	/// <summary>
	/// checks if a type has a userdata unary inus defined via c++ operator.
	/// </summary>
	template<class T>
	concept UnaryMinusOp = std::is_nothrow_copy_constructible_v<T> && requires (const T a) {
		{-a} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata concat defined manually via Concat static member.
	/// </summary>
	template<class T>
	concept ConcatCpp = std::is_same_v<CppFunction, decltype(&T::Concat)> || std::is_same_v<CFunction, decltype(&T::Concat)>;
	/// <summary>
	/// checks if a type has a userdata newindex defined manually via NewIndex static member.
	/// </summary>
	template<class T>
	concept NewIndexCpp = std::is_same_v<CppFunction, decltype(&T::NewIndex)> || std::is_same_v<CFunction, decltype(&T::NewIndex)>;
	/// <summary>
	/// checks if a type has a userdata call defined manually via Call static member.
	/// </summary>
	template<class T>
	concept CallCpp = std::is_same_v<CppFunction, decltype(&T::Call)> || std::is_same_v<CFunction, decltype(&T::Call)>;
	/// <summary>
	/// checks if a type has a userdata index defined manually via Index static member.
	/// </summary>
	template<class T>
	concept IndexCpp = std::is_same_v<CppFunction, decltype(&T::Index)> || std::is_same_v<CFunction, decltype(&T::Index)>;
	/// <summary>
	/// checks if a type has a base class defined for userdata type
	/// </summary>
	template<class T>
	concept BaseDefined = requires {
		typename T::BaseClass;
	};

	/// <summary>
	/// lua reference. just an int, so pass by value preffered.
	/// </summary>
	/// <see cref='lua50::State::Ref'/>
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
		void Work(State L) override {
			std::invoke(Invoke, L);
		}
	};

	/// <summary>
	/// activation record of a lua hook.
	/// just a pointer, so pass by vale prefered.
	/// </summary>
	class ActivationRecord {
		friend class State;
		lua_Debug* ar;
		ActivationRecord(lua_Debug* ar);
	};

	/// <summary>
	/// aka lua_Hook. no type conversion/exception handling. use CppHook if in doubt.
	/// </summary>
	/// <see cref='lua50::CppHook'/>
	/// <see cref='lua50::CppToCHook'/>
	/// <param name="L">lua state</param>
	/// <param name="ar">activation record</param>
	using CHook = void(__cdecl*)(lua_State* L, lua_Debug* ar);
	/// <summary>
	/// lua hook function (when registered, gets called durng lua code execution).
	/// </summary>
	/// <param name="L">lua state</param>
	/// <param name="ar">activation record</param>
	/// <see cref='lua50::CppToCHook'/>
	/// <see cref='lua50::State::Debug_SetHook'/>
	using CppHook = void(*) (State L, ActivationRecord ar);


	/// <summary>
	/// adapts a CppHook to a CHook, doing all the type conversion and exception handling.
	/// </summary>
	/// <param name="l">lua state</param>
	/// <param name="ar">activation record</param>
	/// <see cref='lua50::CppHook'/>
	/// <see cref='lua50::State::Debug_SetHook'/>
	template<CppHook F>
	void CppToCHook(lua_State* l, lua_Debug* ar) {
		if constexpr (CatchExceptions) {
			bool err = false;
			State L{ l }; // trivial, no destructor to call, so its save
			{ // make sure all c++ objects gets their destructor called
				try {
					F(L, ActivationRecord{ ar });
				}
				catch (const std::exception& e) {
					L.PushFString("%s: %s in %s", typeid(e).name(), e.what(), __FUNCSIG__);
					err = true;
				}
				catch (...) {
					L.PushFString("unnown excetion catched in %s", __FUNCSIG__);
					err = true;
				}
			}
			if (err)
				L.Error();
		}
		else {
			State L{ l };
			F(L, ActivationRecord{ ar });
		}
	}

	class PairsHolder;
	class PairsIter;
	class PairsSentinel;
	class IPairsHolder;
	class IPairsIter;

	/// <summary>
	/// <para>represents a lua state.
	/// contains only a pointer, so pass-by-value is prefered.
	/// you need to close this state manually.</para>
	/// <para>the notation [-x,+y,e] is used to indicate changes in the stack:</para>
	/// <para>x is the amount poped from the stack,</para>
	/// <para>y is the amount pushed to the stack</para>
	/// <para>(? is used for an amount that does not depend on the parameters)</para>
	/// <para>(a|b indicates an amount of a or b),</para>
	/// <para>e is an indication of possible exceptions</para>
	/// <para>(- no exception, m memory only, e other exceptions, v throws on purpose).</para>
	/// </summary>
	class State {
	private:
		lua_State* L;

	public:
		/// <summary>
		/// creates a State from a lua_State* (usually from external APIs).
		/// </summary>
		/// <param name="L">state pointer</param>
		State(lua_State* L);
		/// <summary>
		/// opens a new lua state.
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug lib</param>
		State(bool io = true, bool debug = false);

		/// <summary>
		/// gets the lua_State* to pass to external apis.
		/// </summary>
		/// <returns>lua state</returns>
		lua_State* GetState();

		/// <summary>
		/// opens a new lua state (for similarity with luas c api).
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug lib</param>
		/// <returns>new state</returns>
		static State Create(bool io = true, bool debug = false);
		/// <summary>
		/// closes a lua state.
		/// Do not use the state for anything else after calling Close.
		/// </summary>
		void Close();

		/// <summary>
		/// minimum amount of stack space you have available when entering a function. does not include parameters.
		/// </summary>
		constexpr static int MINSTACK = 20;
		/// <summary>
		/// pseudoindex to access the global environment.
		/// </summary>
		constexpr static int GLOBALSINDEX = -10001;
		/// <summary>
		/// pseudoindex to access the registry.
		/// you can store lua values here that you want to access from C++ code, but should not be available to lua.
		/// use light userdata with adresses of something in your code, or strings prefixed with your library name as keys.
		/// integer keys are reserved for the Reference mechanism.
		/// </summary>
		/// <see cref="lua::State::Ref"/>
		constexpr static int REGISTRYINDEX = -10000;
		/// <summary>
		/// passing this to call signals to return all values.
		/// </summary>
		constexpr static int MULTIRET = -1;
		/// <summary>
		/// returns the pseudoindex to access upvalue i.
		/// </summary>
		/// <param name="i">upvalue number</param>
		/// <returns>pseudoindex</returns>
		constexpr static int Upvalueindex(int i)
		{
			return GLOBALSINDEX - i;
		}

		/// <summary>
		/// gets the top of the stack (the highest valid stack position).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns>stack top</returns>
		int GetTop();
		/// <summary>
		/// checks if the stack can grow to top + extra elements.
		/// if it can do so, grows the stack and returns true. if not, returns false.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="extra">positions to grow</param>
		/// <returns>could grow</returns>
		bool CheckStack(int extra);
		/// <summary>
		/// <para>checks if a index represents a valid stack position.</para>
		/// <para>an index is valid, if it points to a stack position lower or equal to top (and not 0).</para>
		/// <para>an index is acceptable, if it points to a stack position lower or equal to the stack space (and not 0). (there is no func to check this)</para>
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="i">index</param>
		/// <returns>is valid</returns>
		bool IsValidIndex(int i);
		/// <summary>
		/// converts an index to a absolte index (not depending on the stack top position).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="i">index</param>
		/// <returns>abs index</returns>
		int ToAbsoluteIndex(int i);

		/// <summary>
		/// sets the stack index to the acceptable index or 0. fills unused positions with nil, or removes now unused space.
		/// <para>[-?,+?,-]</para>
		/// </summary>
		/// <param name="index">new top index</param>
		void SetTop(int index);
		/// <summary>
		/// pushes a copy of something to the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="index">valid index to copy</param>
		void PushValue(int index);
		/// <summary>
		/// removes the stack position index, and shifts elements down to fill the gap.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index to remove (no pseudoindex)</param>
		void Remove(int index);
		/// <summary>
		/// pops the ToS element and inserts it into index, shifting elements up to make a gap.
		/// <para>[-1,+1,-]</para>
		/// </summary>
		/// <param name="index">valid index to insert to (no pseudoindex)</param>
		void Insert(int index);
		/// <summary>
		/// pops the ToS element and replaces index with it.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index to replace</param>
		void Replace(int index);
		/// <summary>
		/// pops num elements from the stack
		/// <para>[-num,+0,-]</para>
		/// </summary>
		/// <param name="num">amount to pop</param>
		void Pop(int num);

		/// <summary>
		/// returns the type of the index (or None if not valid).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptale index to check</param>
		/// <returns>type</returns>
		LType Type(int index);
		/// <summary>
		/// returns if the value at index is nil.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is nil</returns>
		bool IsNil(int index);
		/// <summary>
		/// returns if the value at index is of type boolean.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is bool</returns>
		bool IsBoolean(int index);
		/// <summary>
		/// returns if the value at index is a number or a string convertible to one.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is number</returns>
		bool IsNumber(int index);
		/// <summary>
		/// returns if the value at index is a string or a number (always cnvertible to string).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is string</returns>
		bool IsString(int index);
		/// <summary>
		/// returns if the value at index is of type table.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is table</returns>
		bool IsTable(int index);
		/// <summary>
		/// returns if the value at index is a function (C or lua).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is func</returns>
		bool IsFunction(int index);
		/// <summary>
		/// returns if the value at index is a C function.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is C func</returns>
		bool IsCFunction(int index);
		/// <summary>
		/// returns if the value at index is a userdata (full or light).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is userdata</returns>
		bool IsUserdata(int index);
		/// <summary>
		/// returns if the value at index is a light userdata.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is light userdata</returns>
		bool IsLightUserdata(int index);
		/// <summary>
		/// returns a string containing the type name t.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="t">type</param>
		/// <returns>type string</returns>
		const char* TypeName(LType t);
		/// <summary>
		/// checks equality of 2 values. may call metamethods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>values equals</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool Equal(int i1, int i2);
		/// <summary>
		/// checks primitive equality of 2 values. does not call metametods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>values equals</returns>
		bool RawEqual(int i1, int i2);
		/// <summary>
		/// checks if i1 is smaller than i2. may call metamethods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>smaller than</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool LessThan(int i1, int i2);
		/// <summary>
		/// compares 2 lua values. returns true, if the value at i1 satisfies op when compared with the value at i2.
		/// also returns false, if any of the indices are invalid.
		/// may call metamethods.
		/// <para>[-0,+0,e]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 1</param>
		/// <param name="op">operator</param>
		/// <returns>satisfies</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool Compare(int i1, int i2, ComparisonOperator op);
		/// <summary>
		/// checks if the index is not valid (none) or the value at it is nil.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>is none or nil</returns>
		bool IsNoneOrNil(int idx);

		/// <summary>
		/// converts the value at index to a boolean. nil, false and none evaluate to false, everything else (including 0) to true.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>bool</returns>
		bool ToBoolean(int index);
		/// <summary>
		/// converts the value at index to a number. must be a number or a string convertible to a number, otherise returns 0.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>number</returns>
		Number ToNumber(int index);
		/// <summary>
		/// converts the value at index to an integer. must be a number or a string convertible to a number, otherise returns 0.
		/// <para>equivalent of calling tonumber and casting to int</para>
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>integer</returns>
		Integer ToInteger(int index);
		/// <summary>
		/// converts the value at index to a string. must be a string or a number, otherwise returns nullptr.
		/// the return value might no longer be valid, if the lua value gets removed from the stack.
		/// the string is guranteed to have a ending 0, but other 0es might be in the string.
		/// <para>warning: converts the value on the stack to a string, which might confuse pairs/next</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <param name="len">outputs the string length, if not nullptr</param>
		/// <returns>c string</returns>
		const char* ToString(int index, size_t* len = nullptr);
		/// <summary>
		/// converts the value at index to a CFunction. must be a CFunction, otherwise returns nullptr.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>CFunction</returns>
		CFunction ToCFunction(int index);
		/// <summary>
		/// converts the value at index to a thread (represented via a lua::State). must be a thread, otherwise throws.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>thread as a state</returns>
		/// <exception cref="lua::LuaException">if not a thread</exception>
		State ToThread(int index);
		/// <summary>
		/// converts the value at index to a debugging pointer. must be a userdata, table, thread, or function, otherwise returns nullptr.
		/// only useful for debugging information, cannot be converted back to its original value.
		/// guranteed to be different for different values.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>debug pointer</returns>
		const void* ToPointer(int index);
		/// <summary>
		/// returns the data pointer of the userdata at index. returns the block adress of a full userdata, the pointer of a light userdata or nullptr.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>userdata pointer</returns>
		void* ToUserdata(int index);
		/// <summary>
		/// returns the length of an object. for strings this is the number of bytes (==chars if each char is one byte).
		/// for tables this is one less than the first integer key with a nil value (except if manualy set to something else).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index"></param>
		/// <returns>size</returns>
		size_t ObjLength(int index);

		/// <summary>
		/// pushes a boolean onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="b">bool</param>
		void Push(bool b);
		/// <summary>
		/// pushes a number onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="n">number</param>
		void Push(Number n);
		/// <summary>
		/// pushes an number onto the stack. (integer gets converted to number).
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="i">int</param>
		void Push(Integer i);
		/// <summary>
		/// pushes a string onto the stack. the string ends at the first embedded 0.
		/// lua copies the string, so the buffer passed into this func can be immediately reused.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">string</param>
		void Push(const char* s);
		/// <summary>
		/// pushes a string onto the stack. the string can contain embedded 0s.
		/// lua copies the string, so the buffer passed into this func can be immediately reused.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">string</param>
		/// <param name="l">string length</param>
		void Push(const char* s, size_t l);
		/// <summary>
		/// pushes nil onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		void Push();
		/// <summary>
		/// pushes a CFunction or CClosure (function with upvalues) onto the stack.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <param name="f">function</param>
		/// <param name="nups">number of upvalues</param>
		void Push(CFunction f, int nups = 0);
		/// <summary>
		/// pushes a CFunction or CClosure (function with upvalues) onto the stack.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <param name="F">function</param>
		/// <param name="nups">number of upvalues</param>
		template<CFunction F>
		void Push(int nups = 0)
		{
			Push(F, nups);
		}
		/// <summary>
		/// pushes a CppFunction or CppClosure (function with upvalues) onto the stack.
		/// to create a CClosure, push the initial values for its upvalues onto the stack, and then call this function with the number of upvalues as nups.
		/// <para>[-nups,+1,m]</para>
		/// </summary>
		/// <param name="F">function</param>
		/// <param name="nups">number of upvalues</param>
		template<CppFunction F>
		void Push(int nups = 0)
		{
			Push(&CppToCFunction<F>, nups);
		}
		/// <summary>
		/// pushes a light userdata onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="ud">userdata pointer</param>
		void PushLightUserdata(void* ud);
		/// <summary>
		/// pushes a formatted string onto the stack. similar to snprintf, but with no extra buffer.
		/// <para>the only format specifiers allowed are: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char.</para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="argp">format arguments</param>
		/// <returns>formatted string</returns>
		const char* PushVFString(const char* s, va_list argp);
		/// <summary>
		/// pushes a formatted string onto the stack. similar to snprintf, but with no extra buffer.
		/// <para>the only format specifiers allowed are: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char.</para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="...">format arguments</param>
		/// <returns>formatted string</returns>
		const char* PushFString(const char* s, ...);
		/// <summary>
		/// concatenates the num values at the top of the stack, using the usual semantics. if num is 0, pushes an empty string, if num is 1, does nothing.
		/// <para>[-num,+1,e]</para>
		/// </summary>
		/// <param name="num">number of values to concatenate</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void Concat(int num);
		/// <summary>
		/// performs an arithmetic operation over the 2 values at the top of the stack (or one in case of unary negation), pops the values and pushes the result.
		/// the top values is the second operand.
		/// this function evaluates lua code.
		/// may call metamethods.
		/// <para>[-2|1,+1,e]</para>
		/// </summary>
		/// <param name="op">operator</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void Arithmetic(ArihmeticOperator op);

		/// <summary>
		/// pushes the metatable of the value at index and returns true if there is one. if there is no metatable pushes nothing and returns false.
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to get the metatable from</param>
		/// <returns>has metatable</returns>
		bool GetMetatable(int index);
		/// <summary>
		/// pops a value from the stack and sets it as the metatable of index.
		/// returns false, if it could not set the metatable, but still pops the value.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to set the metatable of</param>
		/// <returns>successfully set</returns>
		bool SetMetatable(int index);

		/// <summary>
		/// creates a new userdata and returns its adress.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s"></param>
		/// <see cref="lua::State:NewUserData"/>
		/// <returns>userdata pointer</returns>
		void* NewUserdata(size_t s);

		/// <summary>
		/// loads a lua chunk via a reader function.
		/// automatically detects text or binary.
		/// reader should return nullptr and set size to 0 to indicate EOF.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="reader">reader function</param>
		/// <param name="ud">data, passed to reader</param>
		/// <param name="chunkname">name of the chunk</param>
		/// <returns>error code</returns>
		ErrorCode Load(const char* (__cdecl* reader)(lua_State*, void*, size_t*), void* ud, const char* chunkname);

		/// <summary>
		/// dumps a lua function at the top of the stack to binary, which can be loaded again via Load.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="writer">writer function</param>
		/// <param name="ud">data, passed to writer</param>
		void Dump(int(__cdecl* writer)(lua_State*, const void*, size_t, void*), void* ud);
		/// <summary>
		/// dumps a lua function at the top of the stack to binary, which can be loaded again via Load.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <returns>binary data of the function</returns>
		std::string Dump();

		/// <summary>
		/// creates a new table and pushes it onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		void NewTable();
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the table at index onto the stack.
		/// may call metamethods.
		/// <para>[-1,+1,e]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void GetTable(int index);
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the table at index onto the stack.
		/// may not call metamethods.
		/// <para>[-1,+1,-]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		void GetTableRaw(int index);
		/// <summary>
		/// pushes the with n associated value in the table at index onto the stack.
		/// may not call metamethods.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		/// <param name="n">key</param>
		void GetTableRaw(int index, int n);
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the table at index. pops both key and value from the stack.
		/// may call metamethods.
		/// <para>[-2,+0,e]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void SetTable(int index);
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the table at index. pops both key and value from the stack.
		/// may not call metamethods.
		/// <para>[-2,+0,m]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		void SetTableRaw(int index);
		/// <summary>
		/// assigns the value at the top of the stack to the key n in the table at index. pops the value from the stack.
		/// may not call metamethods.
		/// <para>[-1,+0,m]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		/// <param name="n">key</param>
		void SetTableRaw(int index, int n);
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the global table. pops both key and value from the stack.
		/// may not call metamethods.
		/// <para>[-2,+0,m]</para>
		/// </summary>
		void SetGlobal();
		/// <summary>
		/// assigns the value at the top of the stack to the key k in the global table. pops the value from the stack.
		/// may not call metamethods.
		/// <para>[-1,+0,m]</para>
		/// </summary>
		/// <param name="k">key</param>
		void SetGlobal(const char* k);
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the global table onto the stack.
		/// may not call metamethods.
		/// <para>[-1,+1,-]</para>
		/// </summary>
		void GetGlobal();
		/// <summary>
		/// pushes the with k associated value in the global table onto the stack.
		/// may not call metamethods.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="k"></param>
		void GetGlobal(const char* k);
		/// <summary>
		/// pushes the global environment (table).
		/// <para>[-0,+1,-]</para>
		/// </summary>
		void PushGlobalTable();
		/// <summary>
		/// traverses the table index by poping the previous key from the stack and pushing the next key and value to the stack.
		/// if there are no more elements in the table, returns false and pushes nothing. otherwise returns true.
		/// <para>do not call tostring onto a key, unless you know that it is acually a string</para>
		/// <para>[-1,+2|0,e]</para>
		/// </summary>
		/// <param name="index">valid index to traverse</param>
		/// <returns>had next</returns>
		bool Next(int index);
		/// <summary>
		/// <para>allows iteration over a lua table.</para>
		/// <para>while iterating, the key is at -2, and the value is at -1.</para>
		/// <para>do not pop value or key.</para>
		/// <para>do not apply ToString directly onto the key, unless you know it is actually a string.</para>
		/// <para>iterator returns the type of the key.</para>
		/// <para>++PairsIter may also throw, if something goes wrong.</para>
		/// <para>when the iteration ends by reaching its end, no key/value pair is left on the stack.</para>
		/// <para>if you break the iteration (or leave it otherwise), you have to clean up the key/value pair from the stack yourself.</para>
		/// <para>[-0,+2|0,e]</para>
		/// </summary>
		/// <param name="index">valid index to iterate over</param>
		/// <returns>PairsHolder</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		PairsHolder Pairs(int index);
		/// <summary>
		/// <para>allows iteration over an array style lua table.</para>
		/// <para>while iterating the key is in the iterator, and the value is at -1.</para>
		/// <para>the iteration begins at key 1 and ends directly before the first key that is assigned nil.</para>
		/// <para>when the iteration ends by reaching its end, no value is left on the stack.</para>
		/// <para>if you break the iteration (or leave it otherwise), you have to clean up the value from the stack yourself.</para>
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="index">valid index to iterate over</param>
		/// <returns>IPairsHolder</returns>
		IPairsHolder IPairs(int index);

		// no access to fenv

		/// <summary>
		/// calls a function. does not catch exceptions, so better use pcall or tcall instead.
		/// first push the function, then the arguments in order, then call.
		/// pops the function and its arguments, then pushes its results.
		/// use MULTIRET to return all values, use GetTop tofigure out how many got returned.
		/// <para>[-nargs+1,+nresults,e]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		void Call(int nargs, int nresults);
		/// <summary>
		/// calls a function. does catch exceptions, and returns an error code.
		/// first push the function, then the arguments in order, then call.
		/// pops the function and its arguments, then pushes its results.
		/// use MULTIRET to return all values, use GetTop tofigure out how many got returned.
		/// if an error gets cought, calls the error handler (if not 0) with an error message, which returns an error message, which then gets pushed onto the stack.
		/// <para>[-nargs+1,+nresults|1,-]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		/// <param name="errfunc">valid index of the error handler function (no pseudoindex) or 0</param>
		/// <returns>error code</returns>
		ErrorCode PCall(int nargs, int nresults, int errfunc = 0);
		/// <summary>
		/// calls a function. does catch lua exceptions, and throws an LuaException.
		/// first push the function, then the arguments in order, then call.
		/// pops the function and its arguments, then pushes its results.
		/// use MULTIRET to return all values, use GetTop tofigure out how many got returned.
		/// if an error gets cought, attaches a stack trace and then throws a LuaException.
		/// <para>[-nargs+1,+nresults|0,-]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		/// <exception cref="lua::LuaException">on lua error</exception>
		void TCall(int nargs, int nresults);
		/// <summary>
		/// turns the value at index to a debug string.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>debug string</returns>
		std::string ToDebugString(int index);
		/// <summary>
		/// generates a stack trace from levelStart to levelEnd (or the end of the stack).
		/// level 0 is the current running function, and n+1 is the function that called n.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="levelStart">stack level to start</param>
		/// <param name="levelEnd">stack level to end (may end before that, if the end of the stack is reached)</param>
		/// <param name="upvalues">include a ToDebugString of upvalues</param>
		/// <param name="locals">include a ToDebugString of locals</param>
		/// <returns>stack trace</returns>
		std::string GenerateStackTrace(int levelStart = 0, int levelEnd = -1, bool upvalues = false, bool locals = false);
		/// <summary>
		/// intended to be used with Pcall. attaches a stack trace to its first parameter.
		/// </summary>
		/// <param name="L">lua state</param>
		/// <returns>1</returns>
		static int DefaultErrorDecorator(State L);
		/// <summary>
		/// returns a string describing the error code c.
		/// </summary>
		/// <param name="c">error code</param>
		/// <returns>error string</returns>
		static const char* ErrorCodeFormat(ErrorCode c);
		// allows to use the lua api without running into lua errors that end up in panic. throws errors as LuaException C++ exception
		void ProtectedAPI(APIProtector* p);
		template<class T>
		requires std::invocable<T, State>
		void ProtectedAPI(T callable) {
			APIProtectorInvoke<T> p{ callable };
			ProtectedAPI(&p);
		}
	private:
		static int ProtectedAPIExecutor(State L);

	public:
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="f">function to register</param>
		/// <param name="index">valid index where to register</param>
		void RegisterFunc(const char* name, CFunction f, int index = GLOBALSINDEX);
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		template<CFunction F>
		void RegisterFunc(const char* name, int index = GLOBALSINDEX) {
			RegisterFunc(name, F, index);
		}
		/// <summary>
		/// registers the function f via the key name in index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="name">key to register</param>
		/// <param name="F">function to register</param>
		/// <param name="index">valid index where to register</param>
		template<CppFunction F>
		void RegisterFunc(const char* name, int index = GLOBALSINDEX)
		{
			RegisterFunc(name, &CppToCFunction<F>, index);
		}
		/// <summary>
		/// registers all functions in funcs into index.
		/// use index = -3 to register in the ToS.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="funcs">iterable containing FuncReference to register</param>
		/// <param name="index">valid index where to register</param>
		template<class T>
		void RegisterFuncs(const T& funcs, int index = GLOBALSINDEX) {
			for (const FuncReference& f : funcs) {
				RegisterFunc(f.Name, f.Func, index);
			}
		}
		/// <summary>
		/// registers all functions in funcs into a global table name.
		/// reuses existing table, if present, creates a new one otherwise.
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="funcs">iterable containing FuncReference to register</param>
		/// <param name="name">name of the global table</param>
		template<class T>
		void RegisterGlobalLib(const T& funcs, const char* name) {
			Push(name);
			Push(name);
			GetGlobal();
			if (!IsTable(-1)) {
				Pop(1);
				NewTable();
			}
			RegisterFuncs(funcs, -3);
			SetGlobal();
		}

		/// <summary>
		/// jumps to the lua error handler (throws a lua error). uses the element at the ToS as error message.
		/// usually its better to use the included C++ to lua error handling.
		/// <para>[-1,+0,v]</para>
		/// </summary>
		[[noreturn]] void Error();

		/// <summary>
		/// creates a new lua thread (coroutine), sharing all global objects, but having an own execution stack.
		/// do not run this in a different c thread, for real multithreading you have to use different states.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <returns></returns>
		State NewThread();
		/// <summary>
		/// resumes a thread (coroutine). to create a thread, push its main function and its arguments and then call resume with the number of arguments.
		/// to restart a thread push the values yield should return onto the stack and call resume with the number of arguments.
		/// this call returns when the thread finishes execution or calls yield.
		/// in case of an error, the stack contains the error message, otherwise the return values of the thread (or parameters passed to yield).
		/// <para>[-?,+?,-]</para>
		/// </summary>
		/// <param name="narg">number of arguments</param>
		/// <returns>error code</returns>
		ErrorCode ResumeThread(int narg);
		/// <summary>
		/// yields a thread and returns to resume. first pass the return values and then call yield with the number of return values.
		/// this function may never return, it is not possible to yield through C boundaries in lua 5.0.
		/// <para>[-?,+?,-]</para>
		/// </summary>
		/// <param name="nret"></param>
		[[noreturn]] void YieldThread(int nret);
		/// <summary>
		/// pops num values from this and pushes them onto to.
		/// may only be used to move values between threads of the same gobal state, not between global states.
		/// (otherwise breaks GC).
		/// <para>[-num,+0,-] on this</para>
		/// <para>[-0,+num,-] on to</para>
		/// </summary>
		/// <param name="to">transfer values to</param>
		/// <param name="num">number ov values to transfer</param>
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
		/// <summary>
		/// gets the debug info for a stack level.
		/// stack level 0 is the current running function, n+1 is the function that called n.
		/// <para>[-0,+0|1,-]</para>
		/// </summary>
		/// <param name="level">stack level to query</param>
		/// <param name="Info">write info into here</param>
		/// <param name="opt">what to query</param>
		/// <param name="pushFunc">push the running function onto the stack.</param>
		/// <returns>level valid</returns>
		bool Debug_GetStack(int level, DebugInfo& Info, DebugInfoOptions opt, bool pushFunc);
		/// <summary>
		/// gets the debug info for a function at ToS.
		/// Pops the function.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="opt">what to query</param>
		/// <returns>debug info</returns>
		DebugInfo Debug_GetInfoForFunc(DebugInfoOptions opt);
		/// <summary>
		/// gets the local value number localnum of the function at the stack level level.
		/// returns the local name and pushes the current value.
		/// returns nullptr and pushes nothing if level or localnum are invalid.
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="level">stack level</param>
		/// <param name="localnum">number of local (1 based)</param>
		/// <returns>local name</returns>
		const char* Debug_GetLocal(int level, int localnum);
		/// <summary>
		/// sets the local value number localnum of the function at the stack level level.
		/// returns the local name and pops the set value.
		/// returns nullptr and pops nothing if level or localnum are invalid.
		/// <para>[-1|0,+0,-]</para>
		/// </summary>
		/// <param name="level">stack level</param>
		/// <param name="localnum">number of local (1 based)</param>
		/// <returns>local name</returns>
		const char* Debug_SetLocal(int level, int localnum);
		/// <summary>
		/// gets the upvalue upnum of the function at index.
		/// returns the upvalue name and pushes the current value.
		/// returns nullptr and pushes nothing, if upnum is invalid.
		/// for C/C++ functions, uses the empty string as name for all valid upvalues.
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="index">valid index to query the upvalue from</param>
		/// <param name="upnum">number of upvalue (1 based)</param>
		/// <returns>upvalue name</returns>
		const char* Debug_GetUpvalue(int index, int upnum);
		/// <summary>
		/// gets the upvalue upnum of the function at index.
		/// returns the upvalue name and pops the set value.
		/// returns nullptr and pops nothing, if upnum is invalid.
		/// for C/C++ functions, uses the empty string as name for all valid upvalues.
		/// <para>[-1|0,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index to set the upvalue of</param>
		/// <param name="upnum">number of upvalue</param>
		/// <returns>upvalue name</returns>
		const char* Debug_SetUpvalue(int index, int upnum);
	private:
		void Debug_SetHook(CHook hook, HookEvent mask, int count);
	public:
		/// <summary>
		/// sets the hook. the hook function gets called every time one of the conditions mask is met.
		/// removes any previous hooks.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="F">hook function</param>
		/// <param name="mask">conditions when to call</param>
		/// <param name="count">count parameter for count condition</param>
		template<CppHook F>
		void Debug_SetHook(HookEvent mask, int count) {
			Debug_SetHook(&CppToCHook<F>, mask, count);
		}
		/// <summary>
		/// removes the currently set hook.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		void Debug_UnSetHook();
		/// <summary>
		/// gets the event that caused the hook to get called from the ar.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="ar">activation record</param>
		/// <returns>event</returns>
		HookEvent Debug_GetEventFromAR(ActivationRecord ar);
		/// <summary>
		/// gets the debug info for the ar.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="ar">ar to get the info from</param>
		/// <param name="opt">what to get</param>
		/// <param name="pushFunc"push the active function></param>
		/// <returns>debug info</returns>
		DebugInfo Debug_GetInfoFromAR(ActivationRecord ar, DebugInfoOptions opt, bool pushFunc = false);
		/// <summary>
		/// gets the current hook.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns>hook</returns>
		CHook Debug_GetHook();
		/// <summary>
		/// gets the current hook mask (conditions).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns>hook event</returns>
		HookEvent Debug_GetHookMask();
		/// <summary>
		/// gets the current hook count.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns>count</returns>
		int Debug_GetHookCount();



		// auxlib (luaL_)

		/// <summary>
		/// gets the string used for a metaevent.
		/// </summary>
		/// <param name="f">event</param>
		/// <returns>event string</returns>
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
			case MetaEvent::ToString:
				return "__tostring";
			case MetaEvent::Name:
				return "__name";
			default:
				return "";
			};
		}

		
		
		/// <summary>
		/// generates an error message of the form
		/// 'bad argument #&lt;arg&gt; to &lt;func&gt; (&lt;extramsg&gt;)'
		/// throws c++ or lua error depending on CatchExceptions.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="arg">argument number</param>
		/// <param name="msg">extra message</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void ArgError(int arg, const char* msg);
		/// <summary>
		/// calls ArgError if b is not satisfied.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="b">condition</param>
		/// <param name="arg">argument number</param>
		/// <param name="msg">extra message</param>
		/// <exception cref="lua::LuaException">if !b</exception>
		void ArgCheck(bool b, int arg, const char* msg);
		/// <summary>
		/// if obj has a metatable and a field ev in it, calls it with obj as its only argument and pushes its return value.
		/// returns if it found a method to call.
		/// <para>[-0,+0|1,e]</para>
		/// </summary>
		/// <param name="obj">valid index to call methamethod of</param>
		/// <param name="ev">event string</param>
		/// <returns>found method</returns>
		bool CallMeta(int obj, const char* ev);
		/// <summary>
		/// if obj has a metatable and a field ev in it, calls it with obj as its only argument and pushes its return value.
		/// returns if it found a method to call.
		/// <para>[-0,+0|1,e]</para>
		/// </summary>
		/// <param name="obj">valid index to call methamethod of</param>
		/// <param name="ev">event</param>
		/// <returns>found method</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		bool CallMeta(int obj, MetaEvent ev);

		/// <summary>
		/// checks if there is any argument including nil) at idx
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <exception cref="lua::LuaException">if none</exception>
		void CheckAny(int idx);
		/// <summary>
		/// checks if there is a number and returns it cast to a int.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		Integer CheckInt(int idx);
		/// <summary>
		/// checks if there is a string and returns it.
		/// <para>warning: converts the value on the stack to a string, which might confuse pairs/next</para>
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="len">optional, writes length of the string here, if not nullptr</param>
		/// <returns>c string</returns>
		/// <exception cref="lua::LuaException">if not string</exception>
		const char* CheckString(int idx, size_t* len = nullptr);
		/// <summary>
		/// checks if there is a number and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>number</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		Number CheckNumber(int idx);
		/// <summary>
		/// checks if there is a number and returns it cast to a float.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>float</returns>
		/// <exception cref="lua::LuaException">if not number</exception>
		float CheckFloat(int idx);
		/// <summary>
		/// checks if there is a bool and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>bool</returns>
		/// <exception cref="lua::LuaException">if not bool</exception>
		bool CheckBool(int idx);
		/// <summary>
		/// checks if the stack can grow to top + extra elements, and does so if possible.
		/// throws if it cannot.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="extra">number of elements</param>
		/// <param name="msg">extra error message</param>
		/// <exception cref="lua::LuaException">if it cannot grow the stack</exception>
		void CheckStack(int extra, const char* msg);
		/// <summary>
		/// checks the type if idx.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="t">type</param>
		/// <exception cref="lua::LuaException">if type does not match</exception>
		void CheckType(int idx, LType t);
		/// <summary>
		/// checks for an userdata type. (via its metatable).
		/// returns nullptr on fail.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="name">metatable name</param>
		/// <returns>userdata</returns>
		/// <see cref="lua::State::GetUserData"/>
		void* CheckUserdata(int idx, const char* name);

		/// <summary>
		/// loads a file as lua code and executes it.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="filename">file name</param>
		/// <returns>error code</returns>
		ErrorCode DoFile(const char* filename);
		/// <summary>
		/// loads a string as lua code and executes it.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="code">lua code</param>
		/// <returns>error code</returns>
		ErrorCode DoString(const char* code);
		/// <summary>
		/// loads a string as lua code and executes it.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="code">lua code</param>
		/// <param name="l">code length</param>
		/// <param name="name">code name</param>
		/// <returns>error code</returns>
		ErrorCode DoString(const char* code, size_t l, const char* name);
		/// <summary>
		/// loads a buffer as lua code and leaves it on the stack to execute.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="code">lua code</param>
		/// <param name="len">code length</param>
		/// <param name="name">code name</param>
		/// <returns>error code</returns>
		ErrorCode LoadBuffer(const char* code, size_t len, const char* name);
		/// <summary>
		/// loads a file as lua code and leaves it on the stack to execute.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="filename">file name</param>
		/// <returns>error code</returns>
		ErrorCode LoadFile(const char* filename);
		/// <summary>
		/// loads a string as lua code and executes it. throws on errors.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="code">code</param>
		/// <param name="len">code lenght</param>
		/// <param name="name">code name</param>
		/// <exception cref="lua::LuaException">on lua exceptions</exception>
		void DoStringT(const char* code, size_t len = 0, const char* name = nullptr);

		/// <summary>
		/// raises a lua error with a formatted string as message.
		/// (usually ThrowLuaFormatted is the better alternative),
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="fmt"></param>
		/// <param name=""></param>
		[[noreturn]] void Error(const char* fmt, ...);
		/// <summary>
		/// throws an error with the message: "location: bad argument narg to 'func' (tname expected, got rt)"
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">index</param>
		/// <param name="t">expected type</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void TypeError(int idx, LType t);
		/// <summary>
		/// throws an error with the message: "location: bad argument narg to 'func' (tname expected, got rt)"
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">index</param>
		/// <param name="t">expected type</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void TypeError(int idx, const char* t);
		/// <summary>
		/// throws an error if a is not met.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="a">condition</param>
		/// <param name="msg">message</param>
		/// <exception cref="lua::LuaException">if !a</exception>
		void Assert(bool a, const char* msg);
		/// <summary>
		/// used to build a prefix for error messages, pushes a string of the form 'chunkname:currentline: '
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="lvl">stack level</param>
		void Where(int lvl);

		/// <summary>
		/// pushes the metafield of obj onto the stack.
		/// returns if it found one, pushes nothing if not.
		/// <para>[-0,+1|0,m]</para>
		/// </summary>
		/// <param name="obj">object to check</param>
		/// <param name="ev">event name</param>
		/// <returns>found</returns>
		bool GetMetaField(int obj, const char* ev);
		/// <summary>
		/// pushes the metafield of obj onto the stack.
		/// returns if it found one, pushes nothing if not.
		/// <para>[-0,+1|0,m]</para>
		/// </summary>
		/// <param name="obj">object to check</param>
		/// <param name="ev">event code</param>
		/// <returns>found</returns>
		bool GetMetaField(int obj, MetaEvent ev);
		/// <summary>
		/// pushes the metatable associated with name.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="name">metatable name</param>
		void GetMetaTableFromRegistry(const char* name);
		/// <summary>
		/// if the registry already has a value associated with name, returns 0. otherwise creates a new table, adds it and returns 1.
		/// in both cases, pushes the final value onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="name">metatable name</param>
		/// <returns>created</returns>
		bool NewMetaTable(const char* name);

		/// <summary>
		/// in idx is a number returns it cast to an int. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>int</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		Integer OptInteger(int idx, Integer def);
		/// <summary>
		/// in idx is a string returns. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>warning: converts the value on the stack to a string, which might confuse pairs/next</para>
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <param name="l">optional length out</param>
		/// <returns>c string</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		const char* OptString(int idx, const char* def, size_t* l = nullptr);
		/// <summary>
		/// in idx is a number returns it. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>number</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		Number OptNumber(int idx, Number def);
		/// <summary>
		/// in idx is a bool returns it. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>bool</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		bool OptBool(int idx, bool def);
		/// <summary>
		/// in idx is a number returns it cast to an float. if idx is none or nil, returns def.
		/// otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">aceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>float</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		float OptFloat(int idx, float def);

		/// <summary>
		/// creates a unique reference to a value.
		/// pops the value.
		/// <para>[-1,+0,m]</para>
		/// </summary>
		/// <param name="t">table to reference in</param>
		/// <returns>reference</returns>
		Reference Ref(int t = REGISTRYINDEX);
		/// <summary>
		/// frees the reference r.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="r">reference</param>
		/// <param name="t">table to reference in</param>
		void UnRef(Reference r, int t = REGISTRYINDEX);
		/// <summary>
		/// pushes the value associated with the reference r.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="r">reference</param>
		/// <param name="t">table to reference in</param>
		void Push(Reference r, int t = REGISTRYINDEX);

		/// <summary>
		/// no valid reference, guranteed to be different from all valid references.
		/// if pushed, pushes nil.
		/// </summary>
		constexpr static Reference NoRef{ Reference::NOREF };
		/// <summary>
		/// reference to nil.
		/// </summary>
		constexpr static Reference RefNil{ Reference::REFNIL };

		/// <summary>
		/// pushes a std::string.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">string to push</param>
		void Push(const std::string& s);
		/// <summary>
		/// converts to a std::string.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">valid index to convert.</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string ToStdString(int idx);
		/// <summary>
		/// checks if idx is a string and returns it.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if not a string</exception>
		std::string CheckStdString(int idx);
		/// <summary>
		/// if idx is a string, returns it. if it is nil or none, returns a copy of def. otherwise throws.
		/// <para>[-0,+0,v]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <param name="def">default value</param>
		/// <returns>string</returns>
		/// <exception cref="lua::LuaException">if invalid</exception>
		std::string OptStdString(int idx, const std::string& def);
		/// <summary>
		/// converts idx to a string, pushes it and returns it.
		/// calls ToString metamethod, if possible.
		/// <para>[-0,+1,e]</para>
		/// </summary>
		/// <param name="idx">valid index to convert</param>
		/// <param name="len">length output, if not nullptr</param>
		/// <returns>c string</returns>
		const char* ConvertToString(int idx, size_t* len = nullptr);
		/// <summary>
		/// converts idx to a string, pushes it and returns it.
		/// calls ToString metamethod, if possible.
		/// <para>[-0,+1,e]</para>
		/// </summary>
		/// <param name="idx">valid index to convert</param>
		/// <returns>string</returns>
		std::string ConvertToStdString(int idx);
		/// <summary>
		/// loads a std::string and executes it. returns an error code.
		/// <para>[-0,+?,m]</para>
		/// </summary>
		/// <param name="code">code</param>
		/// <param name="name">code name</param>
		/// <returns>error code</returns>
		ErrorCode DoString(const std::string& code, const char* name);

		/// <summary>
		/// formats a string via lua formatting.
		/// <para>format options: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="">format parameters</param>
		/// <returns>string</returns>
		std::string LuaFormat(const char* s, ...);
		/// <summary>
		/// formats a string via lua formatting.
		/// <para>format options: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="">format parameters</param>
		/// <returns>string</returns>
		std::string LuaVFormat(const char* s, va_list args);
		/// <summary>
		/// formats a string via lua formatting and throws an error with it as message.
		/// <para>format options: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char</para>
		/// <para>[-0,+0,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="">format parameters</param>
		/// <exception cref="lua::LuaException">always</exception>
		[[noreturn]] void ThrowLuaFormatted(const char* s, ...);

	private:
		constexpr static const char* MethodsName = "Methods";
		constexpr static const char* TypeNameName = "TypeName";
		constexpr static const char* BaseTypeNameName = "BaseTypeName";

		template<class T, class Base>
		requires std::derived_from<T, Base>
		struct UserDataBaseHolder {
			Base* const BaseObj;
			T ActualObj;

			template<class ... Args>
			UserDataBaseHolder(Args&& ... args) : ActualObj(std::forward<Args>(args)...), BaseObj(static_cast<Base*>(&ActualObj)) {
			}
		};

		template<class T>
		static int UserData_Finalizer(State L)
		{
			if constexpr (BaseDefined<T>) {
				L.GetUserData<T>(1); // just use the checks here to validate the argument
				UserDataBaseHolder<T, typename T::BaseClass>* u = static_cast<UserDataBaseHolder<T, typename T::BaseClass>*>(L.ToUserdata(1));
				u->~UserDataBaseHolder<T, typename T::BaseClass>();
			}
			else {
				L.GetUserData<T>(1)->~T();
			}
			return 0;
		}
		template<class T>
		requires EquatableOp<T>
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
		template<class T>
		requires LessThanOp<T>
		static int UserData_LessThanOperator(State L) {
			if (L.GetTop() < 2) {
				L.Push(false);
				return 1;
			}
			T* t = L.OptionalUserData<T>(1);
			T* o = L.OptionalUserData<T>(2);
			if (t && o) {
				L.Push(*t < *o);
				return 1;
			}
			L.Push(false);
			return 1;
		}
		template<class T>
		requires AddOp<T>
		static int UserData_AddOperator(State L) {
			T* t = L.GetUserData<T>(1);
			T* o = L.GetUserData<T>(2);
			L.NewUserData<T>(std::move(*t + *o));
			return 1;
		}
		template<class T>
		requires SubtractOp<T>
		static int UserData_SubtractOperator(State L) {
			T* t = L.GetUserData<T>(1);
			T* o = L.GetUserData<T>(2);
			L.NewUserData<T>(std::move(*t - *o));
			return 1;
		}
		template<class T>
		requires MultiplyOp<T>
		static int UserData_MultiplyOperator(State L) {
			T* t = L.GetUserData<T>(1);
			T* o = L.GetUserData<T>(2);
			L.NewUserData<T>(std::move(*t * *o));
			return 1;
		}
		template<class T>
		requires DivideOp<T>
		static int UserData_DivideOperator(State L) {
			T* t = L.GetUserData<T>(1);
			T* o = L.GetUserData<T>(2);
			L.NewUserData<T>(std::move(*t / *o));
			return 1;
		}
		template<class T>
		requires UnaryMinusOp<T>
		static int UserData_UnaryMinusOperator(State L) {
			T* t = L.GetUserData<T>(1);
			L.NewUserData<T>(std::move(-(*t)));
			return 1;
		}
		template<class T>
		requires IndexCpp<T>
		static int UserData_IndexOperator(State L) {
			T* t = L.GetUserData<T>(1);
			if constexpr (HasLuaMethods<T>) {
				if (L.GetMetaField(1, MethodsName)) {
					L.PushValue(2);
					L.GetTableRaw(-2);
					if (!L.IsNil(-1))
						return 1;
					L.Pop(2); // nil and metatable
				}
			}
			if constexpr (std::is_same_v<CppFunction, decltype(&T::Index)>) {
				return T::Index(L);
			}
			else {
				return T::Index(L.L);
			}
		}

	public:
		/// <summary>
		/// checks if i is an userdata of type T (or able to be cast to T) abd returns it. returns nullptr if not.
		/// </summary>
		/// <typeparam name="T">class type</typeparam>
		/// <param name="i">acceptable index to check</param>
		/// <returns>obj</returns>
		template<class T>
		T* OptionalUserData(int i) {
			if constexpr (BaseDefined<T>) {
				if (Type(i) != LType::Userdata)
					return nullptr;
				if (!GetMetatable(i))
					return nullptr;
				Push(BaseTypeNameName);
				GetTableRaw(-2);
				if (Type(-1) != LType::String) {
					Pop(2);
					return nullptr;
				}
				const char* n = ToString(-1);
				if (strcmp(n, typename_details::type_name<typename T::BaseClass>())) {
					Pop(2);
					return nullptr;
				}
				Pop(2);
				// do not acces ActualObj here, this might be of a different type alltogether
				using base = T::BaseClass;
				struct Holder {
					base* const BaseObj;
				};
				Holder* u = static_cast<Holder*>(ToUserdata(i));
				return dynamic_cast<T*>(u->BaseObj);
			}
			else {
				return static_cast<T*>(CheckUserdata(i, typename_details::type_name<T>()));
			}
		}
		/// <summary>
		/// checks if i is an userdata of type T (or able to be cast to T) abd returns it. throws if not.
		/// </summary>
		/// <typeparam name="T">class type</typeparam>
		/// <param name="i">acceptable index to check</param>
		/// <returns>obj</returns>
		/// <exception cref="lua::LuaException">if type does not match</exception>
		template<class T>
		T* GetUserData(int i) {
			T* t = OptionalUserData<T>(i);
			if (t == nullptr) {
				if constexpr (CatchExceptions)
					ThrowLuaFormatted("no %s at argument %d", typename_details::type_name<T>(), i);
				else
					Error("no %s at argument %d", typename_details::type_name<T>(), i);
			}
			return t;
		}
		/// <summary>
		/// gets the metatable for type T.
		/// </summary>
		/// <typeparam name="T">type to generate metatable for</typeparam>
		template<class T>
		void GetUserDataMetatable() {
			if (NewMetaTable(typename_details::type_name<T>())) {
				if constexpr (IndexCpp<T>) {
					RegisterFunc<UserData_IndexOperator<T>>(GetMetaEventName(MetaEvent::Index), -3);
					if constexpr (HasLuaMethods<T>) {
						Push(MethodsName);
						NewTable();
						RegisterFuncs(T::LuaMethods, -3);
						SetTableRaw(-3);
					}
				}
				else if constexpr (HasLuaMethods<T>) {
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

				if constexpr (LessThanCpp<T>)
					RegisterFunc<T::LessThan>(GetMetaEventName(MetaEvent::LessThan), -3);
				else if constexpr (LessThanOp<T>)
					RegisterFunc<UserData_LessThanOperator<T>>(GetMetaEventName(MetaEvent::LessThan), -3);

				if constexpr (AddCpp<T>)
					RegisterFunc<T::Add>(GetMetaEventName(MetaEvent::Add), -3);
				else if constexpr (AddOp<T>)
					RegisterFunc<UserData_AddOperator<T>>(GetMetaEventName(MetaEvent::Add), -3);

				if constexpr (SubtractCpp<T>)
					RegisterFunc<T::Substract>(GetMetaEventName(MetaEvent::Subtract), -3);
				else if constexpr (AddOp<T>)
					RegisterFunc<UserData_SubtractOperator<T>>(GetMetaEventName(MetaEvent::Subtract), -3);

				if constexpr (MultiplyCpp<T>)
					RegisterFunc<T::Multiply>(GetMetaEventName(MetaEvent::Multiply), -3);
				else if constexpr (MultiplyOp<T>)
					RegisterFunc<UserData_MultiplyOperator<T>>(GetMetaEventName(MetaEvent::Multiply), -3);

				if constexpr (DivideCpp<T>)
					RegisterFunc<T::Divide>(GetMetaEventName(MetaEvent::Divide), -3);
				else if constexpr (DivideOp<T>)
					RegisterFunc<UserData_DivideOperator<T>>(GetMetaEventName(MetaEvent::Divide), -3);

				if constexpr (PowCpp<T>)
					RegisterFunc<T::Pow>(GetMetaEventName(MetaEvent::Pow), -3);

				if constexpr (UnaryMinusCpp<T>)
					RegisterFunc<T::UnaryMinus>(GetMetaEventName(MetaEvent::UnaryMinus), -3);
				else if constexpr (UnaryMinusOp<T>)
					RegisterFunc<UserData_UnaryMinusOperator<T>>(GetMetaEventName(MetaEvent::UnaryMinus), -3);

				if constexpr (ConcatCpp<T>)
					RegisterFunc<T::Concat>(GetMetaEventName(MetaEvent::Concat), -3);

				if constexpr (NewIndexCpp<T>)
					RegisterFunc<T::NewIndex>(GetMetaEventName(MetaEvent::NewIndex), -3);

				if constexpr (CallCpp<T>)
					RegisterFunc<T::Call>(GetMetaEventName(MetaEvent::Call), -3);

				Push(GetMetaEventName(MetaEvent::Name));
				Push(typename_details::type_name<T>());
				SetTableRaw(-3);
				Push(TypeNameName);
				Push(typename_details::type_name<T>());
				SetTableRaw(-3);
				Push(BaseTypeNameName);
				if constexpr (BaseDefined<T>) {
					static_assert(std::derived_from<T, typename T::BaseClass>);
					Push(typename_details::type_name<typename T::BaseClass>());
				}
				else
					Push(typename_details::type_name<T>());
				SetTableRaw(-3);
			}
		}
		template<class T>
		void PrepareUserDataType() {
			GetUserDataMetatable<T>();
			Pop(1);
		}

		/// <summary>
		/// <para>converts a c++ class to a lua userdata. creates a new full userdata and calls the constructor of T, forwarding all arguments.</para>
		/// <para>a class (metatable) for a userdata type is only generated once, and then reused for all userdata of the same type.</para>
		/// <para></para>
		/// <para>lua class generation:</para>
		/// <para>if T::LuaMethods is iterable over LuaReference, registers them all as userdata methods (__index).</para>
		/// <para></para>
		/// <para>if T is not trivially destructable, generates a finalizer (__gc) that calls its destructor.</para>
		/// <para></para>
		/// <para>metatable operator definition:</para>
		/// <para>- by CFunction or CppFunction: you provide an implementation as a static class member (used, if both provided).</para>
		/// <para>- or automatically by C++ operator overloads (this requires a nothrow move constructor for operators).</para>
		/// <para></para>
		/// <para>== comparator (also ~= comparator) (__eq):</para>
		/// <para>- Equals static member.</para>
		/// <para>- == operator overload (or &lt;==&gt; overload) (checks type, then operator).</para>
		/// <para></para>
		/// <para>&lt; comparator (also &gt;, &gt;=, &lt;= comparator) (__lt and __le):</para>
		/// <para>- LessThan static member.</para>
		/// <para>- &lt; operator overload (or &lt;==&gt; overload) (checks type, then operator).</para>
		/// <para></para>
		/// <para>+ operator (__add):</para>
		/// <para>- Add static member.</para>
		/// <para>- + operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>- operator (__sub):</para>
		/// <para>- Substract static member.</para>
		/// <para>- - operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>* operator (__mul):</para>
		/// <para>- Multiply static member.</para>
		/// <para>- * operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>/ operator (__div):</para>
		/// <para>- Divide static member.</para>
		/// <para>- / operator overload (only works for both operands of type T).</para>
		/// <para></para>
		/// <para>^ operator (__pow):</para>
		/// <para>- Pow static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>unary - operator (__unm):</para>
		/// <para>- UnaryMinus static member.</para>
		/// <para>- (unary) - operator overload.</para>
		/// <para></para>
		/// <para>.. operator (__concat):</para>
		/// <para>- Concat static member.</para>
		/// <para>(no operator in c++).</para>
		/// <para></para>
		/// <para>[x]=1 operator (__newindex):</para>
		/// <para>- NewIndex static member.</para>
		/// <para></para>
		/// <para>(...) operator (__call)</para>
		/// <para>- Call static member.</para>
		/// <para></para>
		/// <para>=[x] operator (__index):</para>
		/// <para>- Index static member.</para>
		/// <para></para>
		/// <para>if T has both LuaMethods and Index defined, first LuaMethods is searched, and if nothing is found, Index is called.</para>
		/// <para></para>
		/// <para>to handle inheritance, define T::BaseClass as T in the base class and do not change the typedef in the derived classes.</para>
		/// <para>a call to GetUserData&lt;T::BaseClass&gt; on an userdata of type T will then return a correctly cast pointer to T::BaseClass.</para>
		/// <para>all variables for class generation get used via normal overload resolution, meaning the most derived class wins.</para>
		/// <para>make sure you include all methods from base classes in LuaMethods, or they will get lost.</para>
		/// <para>as far as luapp is concerned a class may only have one base class (defined via T::BaseClass) but other inheritances that are not visible to luapp are allowed.</para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <typeparam name="T">type to create</typeparam>
		/// <param name="...args">parameters for constructor</param>
		/// <returns>obj</returns>
		template<class T, class ... Args>
		T* NewUserData(Args&& ... args) {
			if constexpr (BaseDefined<T>) {
				UserDataBaseHolder<T, typename T::BaseClass>* t = new (NewUserdata(sizeof(UserDataBaseHolder<T, typename T::BaseClass>))) UserDataBaseHolder<T, typename T::BaseClass>(std::forward<Args>(args)...);
				GetUserDataMetatable<T>();
				SetMetatable(-2);
				return &t->ActualObj;
			}
			else {
				T* t = new (NewUserdata(sizeof(T))) T(std::forward<Args>(args)...);
				GetUserDataMetatable<T>();
				SetMetatable(-2);
				return t;
			}
		}
	private:
		static std::string int2Str(int i);
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
			else if (IsFunction(-1)) {
				Push(MethodsName);
				GetTableRaw(-3);
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
				re += "\nindex ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
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

			Push(GetMetaEventName(MetaEvent::LessThan));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\ncomparator lessthan ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Add));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nadd ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Subtract));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nsub ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Multiply));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nmul ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Divide));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\ndiv ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Pow));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\npow ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::UnaryMinus));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nunm ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Concat));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nconcat ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::NewIndex));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\nnewindex ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			Push(GetMetaEventName(MetaEvent::Call));
			GetTableRaw(-2);
			if (IsFunction(-1)) {
				re += "\ncall ";
				re += int2Str(reinterpret_cast<int>(ToPointer(-1)));
			}
			Pop(1);

			re += "\ncomplete";
			Pop(1);
			return re;
		}
	};

	/// <summary>
	/// holds info to iterate over a table. provides begin and end.
	/// </summary>
	class PairsHolder {
		friend class State;
		State L;
		int index;
		PairsHolder(State l, int i);
	public:
		/// <summary>
		/// begins a new iteration. pushes the first key/value pair onto the stack.
		/// </summary>
		/// <returns>iterator</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		PairsIter begin();
		/// <summary>
		/// marks the end of a table iteration.
		/// </summary>
		/// <returns>terator sentinel</returns>
		PairsSentinel end();
	};
	/// <summary>
	/// iterator over a lua table.
	/// </summary>
	class PairsIter {
		friend class State;
		friend class PairsHolder;
		friend bool operator==(const PairsIter& i, PairsSentinel s);
		State L;
		int index;
		bool hasNext = false;
		PairsIter(State L, int i);
	public:
		/// <summary>
		/// advances the iterator to the next key/value pair.
		/// </summary>
		/// <returns>this</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		PairsIter& operator++();
		/// <summary>
		/// advances the iterator to the next key/value pair.
		/// </summary>
		/// <returns>copy of this</returns>
		/// <exception cref="lua::LuaException">on lua error</exception>
		PairsIter operator++(int);
		/// <summary>
		/// gets the type of the key
		/// </summary>
		/// <returns>key type</returns>
		LType operator*();
	};
	/// <summary>
	/// iterator sentinel, to check for end.
	/// </summary>
	class PairsSentinel {
	};
	/// <summary>
	/// checks if an iterator is at the end of its table.
	/// </summary>
	/// <param name="i">iterator</param>
	/// <param name="s">iterator sentinel</param>
	/// <returns>at end</returns>
	bool operator==(const PairsIter& i, PairsSentinel s);
	/// <summary>
	/// checks if an iterator is at the end of its table.
	/// </summary>
	/// <param name="i">iterator</param>
	/// <param name="s">iterator sentinel</param>
	/// <returns>at end</returns>
	bool operator==(PairsSentinel s, const PairsIter& i);

	/// <summary>
	///  holds information to iterate over an array style table. provides begin and end.
	/// </summary>
	class IPairsHolder {
		friend class State;
		State L;
		int index;
		IPairsHolder(State l, int i);
	public:
		/// <summary>
		/// begins a new iteration. pushes the first value onto the stack and provides the first key (1).
		/// </summary>
		/// <returns>iterator</returns>
		IPairsIter begin();
		/// <summary>
		/// mars the end of table.
		/// </summary>
		/// <returns>iterator sentinel</returns>
		PairsSentinel end();
	};
	/// <summary>
	/// iterator to iterate over an array style table.
	/// </summary>
	class IPairsIter {
		friend class State;
		friend class IPairsHolder;
		friend bool operator==(const IPairsIter& i, PairsSentinel s);
		State L;
		int index;
		int key = 1;
		bool hasNext = false;
		IPairsIter(State l, int i);
	public:
		/// <summary>
		/// advances the iterator to the next key.
		/// </summary>
		/// <returns>this</returns>
		IPairsIter& operator++();
		/// <summary>
		/// advances the iterator to the next key.
		/// </summary>
		/// <returns>copy of this</returns>
		IPairsIter operator++(int);
		/// <summary>
		/// acesses the current key.
		/// </summary>
		/// <returns>key</returns>
		int operator*();
	};
	/// <summary>
	/// checks if an iterator is at the end of its table.
	/// </summary>
	/// <param name="i">iterator</param>
	/// <param name="s">iterator sentinel</param>
	/// <returns>at end</returns>
	bool operator==(const IPairsIter& i, PairsSentinel s);
	/// <summary>
	/// checks if an iterator is at the end of its table.
	/// </summary>
	/// <param name="i">iterator</param>
	/// <param name="s">iterator sentinel</param>
	/// <returns>at end</returns>
	bool operator==(PairsSentinel s, const IPairsIter& i);

	/// <summary>
	/// automatically closes a state, when it goes out of scope.
	/// </summary>
	class StateCloser {
		State L;
	public:
		/// <summary>
		/// creates a statecloser from an existing state.
		/// </summary>
		/// <param name="l">state</param>
		StateCloser(State l);
		/// <summary>
		/// creates a statecloser with a new state.
		/// </summary>
		/// <param name="io">open io and os libs</param>
		/// <param name="debug">open debug libs</param>
		StateCloser(bool io = true, bool debug = false);
		~StateCloser();
		/// <summary>
		/// gets the underlying state.
		/// </summary>
		/// <returns>state</returns>
		State GetState();
	};
}

#ifndef LuaVersion
#define LuaVersion 5.0
namespace lua = lua50;
#endif
