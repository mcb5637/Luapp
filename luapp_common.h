#pragma once
#include <stdexcept>

// create a lua::State from this to interact with it
struct lua_State;
struct lua_Debug;

#ifdef _MSC_VER
#define LUAPP_CDECL __cdecl
#define LUAPP_FUNCNAME __FUNCSIG__
#else
#define LUAPP_CDECL
#define LUAPP_FUNCNAME __PRETTY_FUNCTION__
#endif

namespace lua {
	/// <summary>
	/// turn on/off exception handling at compile time
	/// if active, CppToCFunction catches any c++ exceptions and converts them to lua exceptions
	/// (this gets used internally as well)
	/// </summary>
	constexpr bool CatchExceptions = true;
	/// <summary>
	/// enable/disable type checks on API methods, as well as some stack space checks
	/// </summary>
	constexpr bool TypeChecks = true;

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
		/// represents a table value. tables are arrays and dictionaries in one.
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
	/// exception thrown by lua
	/// </summary>
	class LuaException : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	/// <summary>
	/// default number type.
	/// </summary>
	using Number = double;
	/// <summary>
	/// integer type.
	/// </summary>
	using Integer = long long;
	/// <summary>
	/// aka lua_CFunction. no type conversion/exception handling. use CppFunction when in doubt.
	/// </summary>
	/// <see cref='lua50::CppFunction'/>
	/// <see cref='lua50::CppToCFunction'/>
	/// <param name="L">lua state</param>
	/// <returns>number of return values on the stack</returns>
	using CFunction = int(LUAPP_CDECL*) (lua_State* L);

	/// <summary>
	/// aka lua_Hook. no type conversion/exception handling. use CppHook if in doubt.
	/// </summary>
	/// <see cref='lua50::CppHook'/>
	/// <see cref='lua50::CppToCHook'/>
	/// <param name="L">lua state</param>
	/// <param name="ar">activation record</param>
	using CHook = void(LUAPP_CDECL*)(lua_State* L, lua_Debug* ar);
}
