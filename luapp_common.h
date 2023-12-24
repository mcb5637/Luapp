#pragma once
#include <stdexcept>
#include <compare>
#include "constexprTypename.h"

// create a lua::State from this to interact with it
struct lua_State;
struct lua_Debug;

namespace lua {
	/// <summary>
	/// turn on/off exception handling at compile time
	/// if active, CppToCFunction catches any c++ exceptions and converts them to lua exceptions
	/// (this gets used internally as well)
	/// </summary>
	constexpr const bool CatchExceptions = true;
	/// <summary>
	/// enable/disable type checks on API methods, as well as some stack space checks
	/// </summary>
	constexpr const bool TypeChecks = true;

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
	/// exception thrown by lua
	/// </summary>
	class LuaException : public std::runtime_error {
	public:
		inline LuaException(const std::string& what) : std::runtime_error(what) {}
		inline LuaException(const char* what) : std::runtime_error(what) {}
		inline LuaException(const LuaException& other) noexcept : std::runtime_error(other) {}
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
	using CFunction = int(__cdecl*) (lua_State* L);

	/// <summary>
	/// aka lua_Hook. no type conversion/exception handling. use CppHook if in doubt.
	/// </summary>
	/// <see cref='lua50::CppHook'/>
	/// <see cref='lua50::CppToCHook'/>
	/// <param name="L">lua state</param>
	/// <param name="ar">activation record</param>
	using CHook = void(__cdecl*)(lua_State* L, lua_Debug* ar);
}
