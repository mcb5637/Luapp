#pragma once
#include "luapp_common.h"

namespace lua::userdata {
	template<class S>
	using CppFunction = int(*)(S L);

    template<class T>
    concept UserClassOperatorTranslate = T::UserClassOperatorTranslate;

    template<class T>
    concept UserClassMetaMethods = T::UserClassMetaMethods;

    template<class S, auto F, class UC>
    concept Registerable = requires (S L, std::string_view n)
    {
        L.template RegisterFunc<F, UC>(n);
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
	/// checks if a type has additional metamethods to register for a userdata type via LuaMetaMethods.
	/// </summary>
	template<class T>
	concept HasLuaMetaMethods = requires {
		{T::LuaMetaMethods.begin()} -> std::input_iterator;
		{T::LuaMetaMethods.end()} -> std::input_iterator;
	};
	/// <summary>
	/// checks if a type has a userdata equals defined manually via Equals static member.
	/// </summary>
	template<class S, class T>
	concept EquatableCpp = UserClassMetaMethods<T> && Registerable<S, &T::Equals, T>;
	/// <summary>
	/// checks if a type has a userdata equals defined via c++ operator.
	/// </summary>
	template<class T>
	concept EquatableOp = UserClassOperatorTranslate<T> && std::equality_comparable<T>;
	/// <summary>
	/// checks if a type has a userdata lessthan defined manually via LessThan static member.
	/// </summary>
	template<class S, class T>
	concept LessThanCpp = UserClassMetaMethods<T> && Registerable<S, &T::LessThan, T>;
	/// <summary>
	/// checks if a type has a userdata lessthan defined via c++ operator.
	/// </summary>
	template<class T>
	concept LessThanOp = UserClassOperatorTranslate<T> && requires (T a, T b) {
		{a < b} -> std::same_as<bool>;
	};
	/// <summary>
	/// checks if a type has a userdata lessthan equals defined manually via LessOrEquals static member.
	/// </summary>
	template<class S, class T>
	concept LessThanEqualsCpp = UserClassMetaMethods<T> && Registerable<S, &T::LessOrEquals, T>;
	/// <summary>
	/// checks if a type has a userdata lessthan equals defined via c++ operator.
	/// </summary>
	template<class T>
	concept LessThanEqualsOp = UserClassOperatorTranslate<T> && requires (T a, T b) {
		{a <= b} -> std::same_as<bool>;
	};
	/// <summary>
	/// checks if a type has a userdata add defined manually via Add static member.
	/// </summary>
	template<class S, class T>
	concept AddCpp = UserClassMetaMethods<T> && Registerable<S, &T::Add, T>;
	/// <summary>
	/// checks if a type has a userdata add defined via c++ operator.
	/// </summary>
	template<class T>
	concept AddOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a + b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata subtract defined manually via Subtract static member.
	/// </summary>
	template<class S, class T>
	concept SubtractCpp = UserClassMetaMethods<T> && Registerable<S, &T::Subtract, T>;
	/// <summary>
	/// checks if a type has a userdata subtract defined via c++ operator.
	/// </summary>
	template<class T>
	concept SubtractOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a - b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata multiply defined manually via Multiply static member.
	/// </summary>
	template<class S, class T>
	concept MultiplyCpp = UserClassMetaMethods<T> && Registerable<S, &T::Multiply, T>;
	/// <summary>
	/// checks if a type has a userdata multiply defined via c++ operator.
	/// </summary>
	template<class T>
	concept MultiplyOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a* b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata divide defined manually via Divide static member.
	/// </summary>
	template<class S, class T>
	concept DivideCpp = UserClassMetaMethods<T> && Registerable<S, &T::Divide, T>;
	/// <summary>
	/// checks if a type has a userdata divide defined via c++ operator.
	/// </summary>
	template<class T>
	concept DivideOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a / b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata integer divide defined manually via IntegerDivide static member.
	/// </summary>
	template<class S, class T>
	concept IntegerDivideCpp = UserClassMetaMethods<T> && Registerable<S, &T::IntegerDivide, T>;
	/// <summary>
	/// checks if a type has a userdata modulo defined manually via Modulo static member.
	/// </summary>
	template<class S, class T>
	concept ModuloCpp = UserClassMetaMethods<T> && Registerable<S, &T::Modulo, T>;
	/// <summary>
	/// checks if a type has a userdata pow defined manually via Pow static member.
	/// </summary>
	template<class S, class T>
	concept PowCpp = UserClassMetaMethods<T> && Registerable<S, &T::Pow, T>;
	/// <summary>
	/// checks if a type has a userdata unary minus defined manually via UnaryMinus static member.
	/// </summary>
	template<class S, class T>
	concept UnaryMinusCpp = UserClassMetaMethods<T> && Registerable<S, &T::UnaryMinus, T>;
	/// <summary>
	/// checks if a type has a userdata unary minus defined via c++ operator.
	/// </summary>
	template<class T>
	concept UnaryMinusOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a) {
		{-a} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary and defined manually via BitwiseAnd static member.
	/// </summary>
	template<class S, class T>
	concept BitwiseAndCpp = UserClassMetaMethods<T> && Registerable<S, &T::BitwiseAnd, T>;
	/// <summary>
	/// checks if a type has a userdata binary and defined via c++ operator.
	/// </summary>
	template<class T>
	concept BitwiseAndOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a& b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary or defined manually via BitwiseOr static member.
	/// </summary>
	template<class S, class T>
	concept BitwiseOrCpp = UserClassMetaMethods<T> && Registerable<S, &T::BitwiseOr, T>;
	/// <summary>
	/// checks if a type has a userdata binary or defined via c++ operator.
	/// </summary>
	template<class T>
	concept BitwiseOrOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a | b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary xor defined manually via BitwiseXOr static member.
	/// </summary>
	template<class S, class T>
	concept BitwiseXOrCpp = UserClassMetaMethods<T> && Registerable<S, &T::BitwiseXOr, T>;
	/// <summary>
	/// checks if a type has a userdata binary xor defined via c++ operator.
	/// </summary>
	template<class T>
	concept BitwiseXOrOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a^ b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary not defined manually via BitwiseNot static member.
	/// </summary>
	template<class S, class T>
	concept BitwiseNotCpp = UserClassMetaMethods<T> && Registerable<S, &T::BitwiseNot, T>;
	/// <summary>
	/// checks if a type has a userdata binary not defined via c++ operator.
	/// </summary>
	template<class T>
	concept BitwiseNotOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a) {
		{~a} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary shift left defined manually via ShiftLeft static member.
	/// </summary>
	template<class S, class T>
	concept ShiftLeftCpp = UserClassMetaMethods<T> && Registerable<S, &T::ShiftLeft, T>;
	/// <summary>
	/// checks if a type has a userdata binary shift left defined via c++ operator.
	/// </summary>
	template<class T>
	concept ShiftLeftOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a << b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata binary shift right defined manually via ShiftRight static member.
	/// </summary>
	template<class S, class T>
	concept ShiftRightCpp = UserClassMetaMethods<T> && Registerable<S, &T::ShiftRight, T>;
	/// <summary>
	/// checks if a type has a userdata binary shift right defined via c++ operator.
	/// </summary>
	template<class T>
	concept ShiftRightOp = UserClassOperatorTranslate<T> && std::is_nothrow_copy_constructible_v<T> && requires (const T a, const T b) {
		{a >> b} -> std::same_as<T>;
	};
	/// <summary>
	/// checks if a type has a userdata length defined manually via Length static member.
	/// </summary>
	template<class S, class T>
	concept LengthCpp = UserClassMetaMethods<T> && Registerable<S, &T::Length, T>;
	/// <summary>
	/// checks if a type has a userdata concat defined manually via Concat static member.
	/// </summary>
	template<class S, class T>
	concept ConcatCpp = UserClassMetaMethods<T> && Registerable<S, &T::Concat, T>;
	/// <summary>
	/// checks if a type has a userdata newindex defined manually via NewIndex static member.
	/// </summary>
	template<class S, class T>
	concept NewIndexCpp = UserClassMetaMethods<T> && Registerable<S, &T::NewIndex, T>;
	/// <summary>
	/// checks if a type has a userdata call defined manually via Call static member.
	/// </summary>
	template<class S, class T>
	concept CallCpp = UserClassMetaMethods<T> && Registerable<S, &T::Call, T>;
	/// <summary>
	/// checks if a type has a userdata index defined manually via Index static member.
	/// </summary>
	template<class S, class T>
	concept IndexCpp = UserClassMetaMethods<T> && Registerable<S, &T::Index, T>;
	/// <summary>
	/// checks if a type has a userdata tostring defined manually via ToString static member.
	/// </summary>
	template<class S, class T>
    concept ToStringCpp = UserClassMetaMethods<T> && Registerable<S, &T::ToString, T>;
    /// <summary>
    /// checks if a type has a userdata serialize defined manually via Serialize static member.
    /// </summary>
    template<class S, class T>
    concept SerializeCpp = UserClassMetaMethods<T> && Registerable<S, &T::Serialize, T>;

    /// <summary>
    /// checks if a type has base classes defined for userdata type
    /// </summary>
    template<class T>
    concept InheritsDefined = requires {
        typename T::InheritsFrom;
    };
    template<class To>
    struct UserClassCast
    {
        template<class From>
        static To* Cast(void* ud)
        {
            return &static_cast<To&>(*static_cast<From*>(ud));
        }
    };


	template<class State, class T>
	int Finalizer(State L)
	{
		L.template CheckUserClass<T>(1)->~T();
		return 0;
	}
	template<class State, class T>
	requires EquatableOp<T>
    // ReSharper disable once CppDFAConstantFunctionResult
    int EqualsOperator(State L) {
		if (L.GetTop() < 2) {
			L.Push(false);
			return 1;
		}
		T* t = L.template OptionalUserClass<T>(1);
		T* o = L.template OptionalUserClass<T>(2);
		if (t && o) {
			L.Push(*t == *o);
			return 1;
		}
		L.Push(false);
		return 1;
	}
	template<class State, class T>
	requires LessThanOp<T>
    // ReSharper disable once CppDFAConstantFunctionResult
	int LessThanOperator(State L) {
		if (L.GetTop() < 2) {
			L.Push(false);
			return 1;
		}
		T* t = L.template OptionalUserClass<T>(1);
		T* o = L.template OptionalUserClass<T>(2);
		if (t && o) {
			L.Push(*t < *o);
			return 1;
		}
		L.Push(false);
		return 1;
	}
	template<class State, class T>
	requires LessThanOp<T>
    // ReSharper disable once CppDFAConstantFunctionResult
	int LessThanEqualsOperator(State L) {
		if (L.GetTop() < 2) {
			L.Push(false);
			return 1;
		}
		T* t = L.template OptionalUserClass<T>(1);
		T* o = L.template OptionalUserClass<T>(2);
		if (t && o) {
			L.Push(*t <= *o);
			return 1;
		}
		L.Push(false);
		return 1;
	}
	template<class State, class T>
	requires AddOp<T>
	int AddOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t + *o));
		return 1;
	}
	template<class State, class T>
	requires SubtractOp<T>
	int SubtractOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t - *o));
		return 1;
	}
	template<class State, class T>
	requires MultiplyOp<T>
	static int MultiplyOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t * *o));
		return 1;
	}
	template<class State, class T>
	requires DivideOp<T>
	int DivideOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t / *o));
		return 1;
	}
	template<class State, class T>
	requires UnaryMinusOp<T>
	int UnaryMinusOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		L.template NewUserClass<T>(std::move(-(*t)));
		return 1;
	}
	template<class State, class T>
	requires BitwiseAndOp<T>
	int BitwiseAndOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t & *o));
		return 1;
	}
	template<class State, class T>
	requires BitwiseOrOp<T>
	int BitwiseOrOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t | *o));
		return 1;
	}
	template<class State, class T>
	requires BitwiseXOrOp<T>
	int BitwiseXOrOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t ^ *o));
		return 1;
	}
	template<class State, class T>
	requires BitwiseNotOp<T>
	int BitwiseNotOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		L.template NewUserClass<T>(std::move(~(*t)));
		return 1;
	}
	template<class State, class T>
	requires ShiftLeftOp<T>
	int ShiftLeftOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t << *o));
		return 1;
	}
	template<class State, class T>
	requires ShiftRightOp<T>
	int ShiftRightOperator(State L) {
		T* t = L.template CheckUserClass<T>(1);
		T* o = L.template CheckUserClass<T>(2);
		L.template NewUserClass<T>(std::move(*t >> *o));
		return 1;
	}
	template<class State, class T>
	requires IndexCpp<State, T>
	int IndexOperator(State L) {
		L.template CheckUserClass<T>(1);
		if constexpr (HasLuaMethods<T>) {
			if (L.GetMetaField(1, State::MethodsName)) {
				L.PushValue(2);
				L.GetTableRaw(-2);
				if (!L.IsNil(-1))
					return 1;
				L.Pop(2); // nil and metatable
			}
		}
		if constexpr (std::is_same_v<CppFunction<State>, decltype(&T::Index)>) {
			return T::Index(L);
		}
		else if constexpr (std::is_same_v<CFunction, decltype(&T::Index)>) {
			return T::Index(L.L);
		}
		else
		{
			return State::template AutoTranslateAPI<&T::Index, 0, T>(L);
		}
	}
	template<class T>
	concept UserClassUVN = std::same_as<decltype(T::NumberUserValues), const int>;
	template<class T>
	constexpr int UserClassUserValues() {
		if constexpr (UserClassUVN<T>)
			return T::NumberUserValues;
		else
			return 0;
	}
	template<class State>
	constexpr int StateMaxUservalues() {
		if constexpr (!State::Capabilities::Uservalues)
			return 0;
		else if constexpr (!State::Capabilities::ArbitraryUservalues)
			return 1;
		else
			return std::numeric_limits<int>::max();
	}
	template<class State, class T>
	concept UserClassUserValuesValid = UserClassUserValues<T>() <= StateMaxUservalues<State>();

    template<class State, class O, int(O::* F)(State L)>
    int MemberFuncAdaptor(State L) {
        auto* t = L.template CheckUserClass<O>(1);
        return std::invoke(F, t, L);
    }
    template<class State, class O, int(O::* F)(State L) const>
    int MemberFuncAdaptor(State L) {
        auto* t = L.template CheckUserClass<O>(1);
        return std::invoke(F, t, L);
    }

    template<class UC>
    class UserClassChecked
    {
        UC* Data;

    public:
        using UserClass = UC;

        explicit UserClassChecked(UC* Data) : Data(Data) {}

        UC* Get()
        {
            return Data;
        }
        const UC* Get() const
        {
            return Data;
        }

        UC* operator->()
        {
            return Data;
        }
        const UC* operator->() const
        {
            return Data;
        }

        UC& operator*()
        {
            return *Data;
        }
        const UC& operator*() const
        {
            return *Data;
        }
    };

    template<class UC, class... Arg>
    class PushNewUserClass
    {
        std::tuple<Arg...> Params;
    public:
        explicit PushNewUserClass(std::in_place_type_t<UC>, Arg... args) : Params(std::forward<Arg>(args)...) {}

        template<class State>
        void Push(State L)
        {
            auto p = [&]<size_t... I>(std::index_sequence<I...>) {
                L.template NewUserClass<UC>(std::get<I>(Params)...);
            };
            p(std::make_index_sequence<sizeof...(Arg)>{});
        }
    };
}

namespace lua
{
    template<class UC>
    using UserClassChecked = userdata::UserClassChecked<UC>;

    template<class UC, class... Arg>
    using PushNewUserClass = userdata::PushNewUserClass<UC, Arg...>;
}
