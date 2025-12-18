#pragma once
#include <array>
#include <tuple>
#include <utility>
#include <string_view>

namespace lut_details
{
    template<class I>
    struct OptionStringElement
    {
        I Check;
        char Result;

        [[nodiscard]] constexpr bool Applies(I c) const
        {
            return (Check & c) == Check;
        }
    };

    template<class I, I Val, OptionStringElement<I> Opt>
    consteval auto SingleOptionStringElement()
    {
        if constexpr (Opt.Applies(Val))
            return std::tuple<char>(Opt.Result);
        else
            return std::tuple<>();
    }

    template<class I, I Val, OptionStringElement<I>... Opt>
    consteval auto MakeOptionString() {
        auto t = std::tuple_cat(SingleOptionStringElement<I, Val, Opt>()...);
        auto f = [&t]<size_t... Idx>(std::index_sequence<Idx...>) {
            return std::array<char, std::tuple_size_v<decltype(t)> + 1>{std::get<Idx>(t)..., '\0'};
        };

        return f(std::make_index_sequence<std::tuple_size_v<decltype(t)>>());
    }

    template<auto T>
    struct DataHolder {
        static constexpr auto D = T;
    };

    template<class I, I Val, OptionStringElement<I>... Opt>
    consteval std::string_view MakeLUTEntry() {
        using d = DataHolder<MakeOptionString<I, Val, Opt...>()>;
        return std::string_view(d::D.data(), d::D.size()-1);
    }

    template<class I, I Max, OptionStringElement<I>... Opt>
    consteval auto MakeLUT() {

        auto f = []<int... Idx>(std::integer_sequence<int, Idx...>) {
            return std::array<std::string_view, Max+1>{
                MakeLUTEntry<I, Idx, Opt...>()...
            };
        };
        return f(std::make_integer_sequence<I, Max+1>());
    }
}
