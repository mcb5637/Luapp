#pragma once
#include <optional>

#include "luapp51_d.h"
#include "luapp_decorator.h"

#ifndef LuaVersion
#define LuaVersion 5.1
namespace lua {
	using State = decorator::State<v51::State>;
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
	using UniqueState = decorator::UniqueState<v51::State>;
	using Reference = State::Reference;
}
#endif
