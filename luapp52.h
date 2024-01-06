#pragma once
#include <optional>

#include "luapp52_d.h"
#include "luapp_decorator.h"

#ifndef LuaVersion
#define LuaVersion 5.2
namespace lua {
	using State = decorator::State<v52::State>;
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
	using UniqueState = decorator::UniqueState<v52::State>;
}
#endif
