#pragma once

#include <tuple>

namespace lua::func
{
    // as seen on http://functionalcpp.wordpress.com/2013/08/05/function-traits/
    template<class F>
    struct FunctionTraits;

    // function pointer
    template<class R, class... Args>
    struct FunctionTraits<R (*)(Args...)> : FunctionTraits<R(Args...)>
    {
    };

    template<class R, class... Args>
    struct FunctionTraits<R(Args...)>
    {
        using ReturnType = R;

        static constexpr std::size_t Arity = sizeof...(Args);

        using ArgumentTypes = std::tuple<Args...>;
        template<std::size_t I>
        using ArgumentType = std::tuple_element_t<I, ArgumentTypes>;
    };

    // member function pointer
    template<class C, class R, class... Args>
    struct FunctionTraits<R (C::*)(Args...)> : FunctionTraits<R(C*, Args...)>
    {
    };

    // const member function pointer
    template<class C, class R, class... Args>
    struct FunctionTraits<R (C::*)(Args...) const> : FunctionTraits<R(C*, Args...)>
    {
    };

    namespace detail
    {
        template<class F>
        struct IsFunctionPointer : std::false_type
        {
        };

        template<class R, class... Args>
        struct IsFunctionPointer<R (*)(Args...)> : std::true_type
        {
        };
    }

    template<class T>
    concept IsFunctionPointer = detail::IsFunctionPointer<T>::value;

    template<class T>
    concept IsMemberFunctionPointer = std::is_member_function_pointer_v<T>;

    namespace detail
    {
        template<class State, class P>
        concept Pushable = requires(State s, P p) { s.Push(p); };

        template<class State, class R>
        concept Checkable = requires(State s, R r, int i) {
            { s.template Check<R>(i) } -> std::same_as<R>;
        };

        template<class T>
        concept IsTuple = requires {
            { std::tuple_size<T>::value } -> std::convertible_to<std::size_t>;
        };

        template<class State, class F, std::size_t NumBindings = 0>
        concept AutoTranslateEnabled =
            (Pushable<State, typename FunctionTraits<F>::ReturnType> ||
             std::same_as<typename FunctionTraits<F>::ReturnType, void> ||
             (IsTuple<typename FunctionTraits<F>::ReturnType> &&
              []<std::size_t... Is>(std::index_sequence<Is...>)
              {
                  return (Pushable<State, std::tuple_element_t<Is, typename FunctionTraits<F>::ReturnType>> && ...);
              }(std::make_index_sequence<std::tuple_size_v<typename FunctionTraits<F>::ReturnType>>{}))) &&
            []<std::size_t... Is>(std::index_sequence<Is...>)
        {
            return ((Is < NumBindings
                         ? std::is_pointer_v<typename FunctionTraits<F>::template ArgumentType<Is>>
                         : Checkable<State, typename FunctionTraits<F>::template ArgumentType<Is>>) &&
                    ...);
        }(std::make_index_sequence<FunctionTraits<F>::Arity>{});
    } // namespace detail
} // namespace lua::func
