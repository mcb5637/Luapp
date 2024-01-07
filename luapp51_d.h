#pragma once
#include <optional>

#include "luapp_common.h"

namespace lua::decorator {
	template<class B>
	class State;
}
namespace lua::v51 {
	using ExConverterT = std::string(*)(std::exception_ptr ex, const char* funcsig);
	/// <summary>
	/// called in exception conversion from c++ to lua (that do not inherit from std::exception).
	/// return value gets thrown as lua exception.
	/// should throw anything, if exception cannot get converted, a default message gets thrown in that case.
	/// </summary>
	extern ExConverterT ExceptionConverter;

	/// <summary>
	/// error codes used by lua.
	/// </summary>
	enum class ErrorCode : int {
		/// <summary>
		/// no error.
		/// </summary>
		Success = 0,
		/// <summary>
		/// thread yielded (paused).
		/// </summary>
		Yield = 1,
		/// <summary>
		/// lua error at runtime.
		/// </summary>
		Runtime = 2,
		/// <summary>
		/// syntax error parsing lua code.
		/// </summary>
		Syntax = 3,
		/// <summary>
		/// out of memory.
		/// </summary>
		Memory = 4,
		/// <summary>
		/// error processing an error handler.
		/// </summary>
		ErrorHandler = 5,
		/// <summary>
		/// IO error reading or writing files.
		/// </summary>
		File,
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
		/// % operator.
		/// </summary>
		Modulo,
		/// <summary>
		/// unary - operator.
		/// </summary>
		UnaryMinus,
		/// <summary>
		/// .. operator.
		/// </summary>
		Concat,
		/// <summary>
		/// # operator.
		/// </summary>
		Length,
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
		/// What, Source, LineDefined, LastLineDefined, ShortSrc fields.
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
		int LastLineDefined = 0;
		char ShortSrc[SHORTSRC_SIZE] = {};
	private:
		int CallInfo = 0;

		friend class State;
		friend void CopyDebugInfo(const lua_Debug& src, DebugInfo& trg);
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

	/// <summary>
	/// activation record of a lua hook.
	/// just a pointer, so pass by vale prefered.
	/// </summary>
	class ActivationRecord {
		template<class B>
		friend class decorator::State;
		friend class State;
		lua_Debug* ar;
		ActivationRecord(lua_Debug* ar);

	public:
		/// <summary>
		/// returns the event that caused the hook call.
		/// </summary>
		/// <returns></returns>
		HookEvent Event() const;
		/// <summary>
		/// returns the line of a line hook event.
		/// </summary>
		/// <returns></returns>
		int Line() const;
		/// <summary>
		/// checks, if the event that caused the hook call is one of the specified events.
		/// </summary>
		/// <param name="e"></param>
		/// <returns></returns>
		bool Matches(HookEvent e) const;
	};

	class State {
	protected:
		lua_State* L;

	public:
		static inline ExConverterT& GetExConv() {
			return ExceptionConverter;
		}

		/// <summary>
		/// lists the capabilities of this lua version.
		/// </summary>
		struct Capabilities {
			/// <summary>
			/// if true, supports lua::Integer natively (not converting them to lua::Number internally), as well as bit operators.
			/// </summary>
			static constexpr bool NativeIntegers = false;
			/// <summary>
			/// if true, supports Debug_UpvalueID and Debug_UpvalueJoin.
			/// </summary>
			static constexpr bool UpvalueId = false;
			/// <summary>
			/// if true, has State::GLOBALSINDEX to directly access globals. if false, it needs to be queried via State::REGISTRY_GLOBALS from the registry.
			/// <para>note that in both cases, functions like State::SetGlobal are provided.</para>
			/// </summary>
			static constexpr bool GlobalsIndex = true;
			/// <summary>
			/// if true, lua::MetaEvent::Length and lua::MetaEvent::Modulo are available, as well as the % operator (instead of math.mod).
			/// </summary>
			static constexpr bool MetatableLengthModulo = true;
			/// <summary>
			/// if true, State::ObjLength calls lua::MetaEvent::Length for tables.
			/// </summary>
			static constexpr bool MetatableLengthOnTables = false;
			/// <summary>
			/// if true, supports at least one uservalue per userdata (might technically be a environment).
			/// </summary>
			static constexpr bool Uservalues = true;
			/// <summary>
			/// if true, supports a fixed number of uservalues per userdata, specified at userdata creation.
			/// </summary>
			static constexpr bool ArbitraryUservalues = false;
			/// <summary>
			/// if true, supports closable slots.
			/// </summary>
			static constexpr bool CloseSlots = false;
			/// <summary>
			/// if true, supports State::REGISTRY_LOADED_TABLE.
			/// </summary>
			static constexpr bool LoadedTable = false;
			/// <summary>
			/// if true, supports State::SetJITMode functions.
			/// </summary>
			static constexpr bool JIT = false;
			/// <summary>
			/// if true, supports State::SetEnvironment and State::GetEnvironment for lua functions.
			/// </summary>
			static constexpr bool Environments = true;
			/// <summary>
			/// if true, supports State::SetEnvironment and State::GetEnvironment for c functions, threads and userdata.
			/// </summary>
			static constexpr bool NonFunctionEnvironments = true;
		};
		using ErrorCode = ErrorCode;
		using ComparisonOperator = ComparisonOperator;
		using ArihmeticOperator = ArihmeticOperator;
		using DebugInfo = DebugInfo;
		using DebugInfoOptions = DebugInfoOptions;
		using MetaEvent = MetaEvent;
		using ActivationRecord = ActivationRecord;
		using HookEvent = HookEvent;

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
		constexpr static int GLOBALSINDEX = -10002;
		/// <summary>
		/// pseudoindex to access the environment of the current running c function.
		/// </summary>
		constexpr static int ENVIRONINDEX = -10001;
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
		/// converts an index to a absolute index (not depending on the stack top position).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="i">index</param>
		/// <returns>abs index</returns>
		int ToAbsoluteIndex(int i);
		/// <summary>
		/// checks if a index is a pseudoindex.
		/// </summary>
		/// <param name="i">index</param>
		/// <returns>is pseudoindex</returns>
		constexpr static bool IsPseudoIndex(int i) {
			return i <= REGISTRYINDEX;
		}

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
		/// copies the element at from to to, replacing it without shifting other elements.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="from">valid index to copy</param>
		/// <param name="to">valid index to replace</param>
		void Copy(int from, int to);
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
		/// returns if the value at index is none (outside the stack)..
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to check</param>
		/// <returns>is none</returns>
		bool IsNone(int index);
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
		/// checks primitive equality of 2 values. does not call metametods.
		/// also returns false, if any of the indices are invalid.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="i1">acceptable index 1</param>
		/// <param name="i2">acceptable index 2</param>
		/// <returns>values equals</returns>
		bool RawEqual(int i1, int i2);
		/// <summary>
		/// checks if the index is not valid (none) or the value at it is nil.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx">acceptable index to check</param>
		/// <returns>is none or nil</returns>
		bool IsNoneOrNil(int idx);

	protected:
		static int Compare_Unprotected(lua_State* L);

	public:
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
		std::optional<Number> ToNumber(int index);
		/// <summary>
		/// converts the value at index to an integer. must be a number or a string convertible to a number, otherise returns 0.
		/// if the number is not an integer, it is truncated.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">acceptable index to convert</param>
		/// <returns>integer</returns>
		std::optional<Integer> ToInteger(int index);
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
		/// for full userdata, it is the size of the allocated block of memory.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index to query</param>
		/// <returns>size</returns>
		size_t RawLength(int index);

	protected:
		static int ObjLen_Unprotected(lua_State* L);

	public:
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
		/// pushes a light userdata onto the stack.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="ud">userdata pointer</param>
		void PushLightUserdata(void* ud);
		/// <summary>
		/// pushes a formatted string onto the stack. similar to snprintf, but with no extra buffer.
		/// <para>the only format specifiers allowed are: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char, %p pointer as hex number</para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="argp">format arguments</param>
		/// <returns>formatted string</returns>
		const char* PushVFString(const char* s, va_list argp);
		/// <summary>
		/// pushes a formatted string onto the stack. similar to snprintf, but with no extra buffer.
		/// <para>the only format specifiers allowed are: %% escape %, %s string (zero-terminated), %f Number, %d int, %c int as single char, %p pointer as hex number</para>
		/// <para>[-0,+1,m]</para>
		/// </summary>
		/// <param name="s">format string</param>
		/// <param name="...">format arguments</param>
		/// <returns>formatted string</returns>
		const char* PushFString(const char* s, ...);
	protected:
		static int Concat_Unprotected(lua_State* L);
		static int Arithmetic_Unprotected(lua_State* L);

	public:
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
		/// pushes the environment of an userdata and returns its type.
		/// if the uservalue does not exist, pushes nil and returns None.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="index">valid index of the userdata</param>
		/// <returns>type</returns>
		LType GetUserValue(int index);
		/// <summary>
		/// pops a value from the stack and sets it as the environment of an userdata.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index of the userdata</param>
		void SetUserValue(int index);

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
		/// creates a new table and pushes it onto the stack.
		/// <para>[-0,+1,m]</para>
		/// </summary>
		void NewTable();
	protected:
		static int GetTable_Unprotected(lua_State* L);
		static int SetTable_Unprotected(lua_State* L);
	public:
		/// <summary>
		/// pops a key from the stack, and pushes the associated value in the table at index onto the stack.
		/// may not call metamethods.
		/// <para>[-1,+1,t]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		void GetTableRaw(int index);
		/// <summary>
		/// pushes the with n associated value in the table at index onto the stack.
		/// may not call metamethods.
		/// <para>[-0,+1,t]</para>
		/// </summary>
		/// <param name="index">valid index for table access</param>
		/// <param name="n">key</param>
		void GetTableRaw(int index, int n);
		/// <summary>
		/// assigns the value at the top of the stack to the key just below the top in the table at index. pops both key and value from the stack.
		/// may not call metamethods.
		/// <para>[-2,+0,mt]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		void SetTableRaw(int index);
		/// <summary>
		/// assigns the value at the top of the stack to the key n in the table at index. pops the value from the stack.
		/// may not call metamethods.
		/// <para>[-1,+0,mt]</para>
		/// </summary>
		/// <param name="index">valid index for table acccess</param>
		/// <param name="n">key</param>
		void SetTableRaw(int index, int n);
		/// <summary>
		/// pushes the global environment (table).
		/// <para>[-0,+1,-]</para>
		/// </summary>
		void PushGlobalTable();
	protected:
		static int Next_Unproteced(lua_State* L);

	public:
		/// <summary>
		/// pushes the environment of the function/thread/userdata at idx.
		/// <para>[-0,+1,-]</para>
		/// </summary>
		/// <param name="idx"></param>
		void GetEnvironment(int idx);
		/// <summary>
		/// sets the table at the top of the stack as the environment of the function/thread/userdata at idx and pops it.
		/// if idx does not have an environment, returns false.
		/// <para>[-1,+0,-]</para>
		/// </summary>
		/// <param name="idx"></param>
		bool SetEnvironment(int idx);

		/// <summary>
		/// calls a function. does not catch exceptions, so better use pcall or tcall instead.
		/// first push the function, then the arguments in order, then call.
		/// pops the function and its arguments, then pushes its results.
		/// use MULTIRET to return all values, use GetTop tofigure out how many got returned.
		/// <para>[-nargs+1,+nresults,et]</para>
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
		/// <para>[-nargs+1,+nresults|1,t]</para>
		/// </summary>
		/// <param name="nargs">number of parameters</param>
		/// <param name="nresults">number of return values</param>
		/// <param name="errfunc">valid index of the error handler function (no pseudoindex) or 0</param>
		/// <returns>error code</returns>
		ErrorCode PCall(int nargs, int nresults, int errfunc = 0);
		/// <summary>
		/// returns a string describing the error code c.
		/// </summary>
		/// <param name="c">error code</param>
		/// <returns>error string</returns>
		static const char* ErrorCodeFormat(ErrorCode c);

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
		/// <para>[-?,+?,t]</para>
		/// </summary>
		/// <param name="narg">number of arguments</param>
		/// <returns>error code</returns>
		ErrorCode ResumeThread(int narg);
		/// <summary>
		/// yields a thread and returns to resume. first pass the return values and then call yield with the number of return values.
		/// this function may never return, it is not possible to yield through C boundaries in lua 5.0.
		/// <para>[-?,+?,t]</para>
		/// </summary>
		/// <param name="nret"></param>
		[[noreturn]] void YieldThread(int nret);
		/// <summary>
		/// pops num values from this and pushes them onto to.
		/// may only be used to move values between threads of the same gobal state, not between global states.
		/// (otherwise breaks GC).
		/// <para>[-num,+0,t] on this</para>
		/// <para>[-0,+num,-] on to</para>
		/// </summary>
		/// <param name="to">transfer values to</param>
		/// <param name="num">number ov values to transfer</param>
		void XMove(State to, int num);
		/// <summary>
		/// returns the lua version number
		/// </summary>
		/// <returns>version</returns>
		static Number Version();

		/// <summary>
		/// checks if the stack level is valid (has a active function).
		/// <para>[-0,+0|1,-]</para>
		/// </summary>
		/// <param name="lvl"></param>
		/// <returns></returns>
		bool Debug_IsStackLevelValid(int lvl);
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
		/// pushes the function associated with a DebugInfo onto the stack.
		/// does not work with Debug_GetInfoForFunc.
		/// <para>[+1,+0,-]</para>
		/// </summary>
		/// <param name="info"></param>
		void Debug_PushDebugInfoFunc(const DebugInfo& info);
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
		/// gets the local value number localnum of the function at the stack level level.
		/// returns the local name and pushes the current value.
		/// returns nullptr and pushes nothing if level or localnum are invalid.
		/// <para>[-0,+1|0,-]</para>
		/// </summary>
		/// <param name="info">stack level</param>
		/// <param name="localnum">number of local (1 based)</param>
		/// <returns>local name</returns>
		const char* Debug_GetLocal(const DebugInfo& info, int localnum);
		/// <summary>
		/// sets the local value number localnum of the function at the stack level level.
		/// returns the local name and pops the set value.
		/// returns nullptr and pops nothing if level or localnum are invalid.
		/// <para>[-1|0,+0,-]</para>
		/// </summary>
		/// <param name="info">stack level</param>
		/// <param name="localnum">number of local (1 based)</param>
		/// <returns>local name</returns>
		const char* Debug_SetLocal(const DebugInfo& info, int localnum);
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
	protected:
		void Debug_SetHook(CHook hook, HookEvent mask, int count);
	public:
		/// <summary>
		/// removes the currently set hook.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		void Debug_UnSetHook();
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
			case MetaEvent::Modulo:
				return "__mod";
			case MetaEvent::Pow:
				return "__pow";
			case MetaEvent::UnaryMinus:
				return "__unm";
			case MetaEvent::Concat:
				return "__concat";
			case MetaEvent::Length:
				return "__len";
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

	private:
		void CheckStackHasElements(int n);

	public:
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

	protected:
		int RefI(int t);
		void UnRefI(int r, int t);
		constexpr static int NOREFI = -2;
		constexpr static int REFNILI = -1;
	};
}
