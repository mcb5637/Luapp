#pragma once

#include "luappjit_d.h"
#include "luapp_decorator.h"

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
	using Reference = State::Reference;
}
#endif
