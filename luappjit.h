#pragma once

#ifndef LuaVersion
#define LuaIsJIT
#define LuaVersion 5.1
#endif

#include "luapp51.h"

namespace lua::jit {
	class State : public lua::v51::State {
	public:
		struct Capabilities {
			static constexpr bool NativeIntegers = false, UpvalueId = true, GlobalsIndex = true, MetatableLengthModulo = true, Uservalues = false, CloseSlots = false, LoadedTable = false, JIT = true;
		};

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
	};
}

#ifdef LuaIsJIT
namespace lua {
	using State = decorator::State<jit::State>;
	using CppFunction = State::CppFunction;
	using ComparisonOperator = State::ComparisonOperator;
	using ArihmeticOperator = State::ArihmeticOperator;
	using DebugInfo = State::DebugInfo;
	using DebugInfoOptions = State::DebugInfoOptions;
	using MetaEvent = State::MetaEvent;
	using FuncReference = State::FuncReference;
	using ErrorCode = State::ErrorCode;
	using ActivationRecord = State::ActivationRecord;
	using HookEvent = State::HookEvent;
	using UniqueState = decorator::UniqueState<jit::State>;
}
#endif
