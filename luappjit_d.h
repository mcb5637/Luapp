#pragma once

#ifndef LuaVersion
#define LuaIsJIT
#define LuaVersion 5.1
#endif

#include "luapp51_d.h"

namespace lua::jit {
	enum class JITMode : int {
		Off = 0,
		On = 0x100,
		Flush = 0x200,
	};
	class State : public lua::v51::State {
	public:
		/// <summary>
		/// lists the capabilities of this lua version.
		/// </summary>
		struct Capabilities {
			/// <summary>
			/// if true, supports lua::Integer natively (not converting them to lua::Number internally), as well as bit operators.
			/// </summary>
			static constexpr bool NativeIntegers = lua::v51::State::Capabilities::NativeIntegers;
			/// <summary>
			/// if true, supports Debug_UpvalueID and Debug_UpvalueJoin.
			/// </summary>
			static constexpr bool UpvalueId = true;
			/// <summary>
			/// if true, has State::GLOBALSINDEX to directly access globals. if false, it needs to be queried via State::REGISTRY_GLOBALS from the registry.
			/// <para>note that in both cases, functions like State::SetGlobal are provided.</para>
			/// </summary>
			static constexpr bool GlobalsIndex = lua::v51::State::Capabilities::GlobalsIndex;
			/// <summary>
			/// if true, lua::MetaEvent::Length and lua::MetaEvent::Modulo are available, as well as the % operator (instead of math.mod).
			/// </summary>
			static constexpr bool MetatableLengthModulo = lua::v51::State::Capabilities::MetatableLengthModulo;
			/// <summary>
			/// if true, State::ObjLength calls lua::MetaEvent::Length for tables.
			/// </summary>
			static constexpr bool MetatableLengthOnTables = lua::v51::State::Capabilities::MetatableLengthOnTables;
			/// <summary>
			/// if true, supports at least one uservalue per userdata (might technically be a environment).
			/// </summary>
			static constexpr bool Uservalues = lua::v51::State::Capabilities::Uservalues;
			/// <summary>
			/// if true, supports a fixed number of uservalues per userdata, specified at userdata creation.
			/// </summary>
			static constexpr bool ArbitraryUservalues = lua::v51::State::Capabilities::ArbitraryUservalues;
			/// <summary>
			/// if true, supports closable slots.
			/// </summary>
			static constexpr bool CloseSlots = lua::v51::State::Capabilities::CloseSlots;
			/// <summary>
			/// if true, supports State::REGISTRY_LOADED_TABLE.
			/// </summary>
			static constexpr bool LoadedTable = lua::v51::State::Capabilities::LoadedTable;
			/// <summary>
			/// if true, supports State::SetJITMode functions.
			/// </summary>
			static constexpr bool JIT = true;
			/// <summary>
			/// if true, supports State::SetEnvironment and State::GetEnvironment for lua functions.
			/// </summary>
			static constexpr bool Environments = lua::v51::State::Capabilities::Environments;
			/// <summary>
			/// if true, supports State::SetEnvironment and State::GetEnvironment for c functions, threads and userdata.
			/// </summary>
		    static constexpr bool NonFunctionEnvironments = lua::v51::State::Capabilities::NonFunctionEnvironments;
		    /// <summary>
		    /// if true, supports State::PushExternalString.
		    /// </summary>
		    static constexpr bool ExternalString = false;
		};
		using JITMode = JITMode;

		/// <summary>
        /// creates a State from a lua_State* (usually from external APIs).
        /// </summary>
        /// <param name="L">state pointer</param>
        explicit State(lua_State* L);
		/// <summary>
        /// opens a new lua state.
        /// </summary>
        /// <param name="io">open io and os libs</param>
        /// <param name="debug">open debug lib</param>
        explicit State(bool io = true, bool debug = false);

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
		/// returns if a coroutine can yield.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <returns>yieldable</returns>
		bool IsYieldable();
		/// <summary>
		/// returns the lua version number
		/// </summary>
		/// <returns>version</returns>
		static Number Version();

		/// <summary>
		/// allows to check if upvalues of (possibly different) functions share the same upvalue.
		/// shared upvalues return the same identifier.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="index">valid index to set the upvalue of</param>
		/// <param name="upnum">number of upvalue, needs to be valid</param>
		/// <returns>upvalue identifier</returns>
		const void* Debug_UpvalueID(int index, int upnum);
		/// <summary>
		/// makes the upMod upvalue of funcMod refer to the upTar upvalue of funcTar.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="funcMod">valid index to modify the upvalue of</param>
		/// <param name="upMod">number of upvalue to modify, needs to be valid</param>
		/// <param name="funcTar">valid index to target the upvalue of</param>
		/// <param name="upTar">number of upvalue to target, needs to be valid</param>
		void Debug_UpvalueJoin(int funcMod, int upMod, int funcTar, int upTar);

		/// <summary>
		/// sets the JIT mode for the whole engine.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="m"></param>
		/// <exception cref="lua::LuaException">if failed</exception>
		void SetJITMode(JITMode m);
		/// <summary>
		/// sets the JIT mode for the function at idx (or the parent of the caller if 0).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx"></param>
		/// <param name="m"></param>
		/// <exception cref="lua::LuaException">if failed</exception>
		void SetJITModeForSingleFunc(int idx, JITMode m);
		/// <summary>
		/// sets the JIT mode for the function at idx (or the parent of the caller if 0) and everything called by it.
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx"></param>
		/// <param name="m"></param>
		/// <exception cref="lua::LuaException">if failed</exception>
		void SetJITModeForFuncAndChildren(int idx, JITMode m);
		/// <summary>
		/// sets the JIT mode for everything called by the function at idx (or the parent of the caller if 0).
		/// <para>[-0,+0,-]</para>
		/// </summary>
		/// <param name="idx"></param>
		/// <param name="m"></param>
		/// <exception cref="lua::LuaException">if failed</exception>
		void SetJITModeForChildrenOnly(int idx, JITMode m);
	};
}
