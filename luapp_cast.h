#pragma once
#include <optional>
#include <concepts>
#include <limits>

namespace lua::cast_detail
{
    template<class To, class From>
    requires (std::integral<To> || std::floating_point<To>) && (std::integral<From> || std::floating_point<From>)
    constexpr std::optional<To> TryCast(From f)
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdouble-promotion"
#endif
        if constexpr (std::floating_point<From> && !std::floating_point<To>) {
            // from https://stackoverflow.com/questions/25857843/how-do-i-convert-an-arbitrary-double-to-an-integer-while-avoiding-undefined-beha
            if (std::isnan(f) || std::isinf(f)) {
                return std::nullopt;
            }
            if constexpr (!std::numeric_limits<To>::is_signed) {
                if (f < 0) {
                    return std::nullopt;
                }
            }
            int exp;
            std::frexp(f, &exp);
            if (exp > std::numeric_limits<To>::digits && f != std::numeric_limits<To>::min()) {
                return std::nullopt;
            }
        } else if constexpr (std::numeric_limits<To>::is_signed == std::numeric_limits<From>::is_signed) {
            if (std::numeric_limits<To>::min() > f || f > std::numeric_limits<To>::max()) {
                return std::nullopt;
            }
        } else if constexpr (std::numeric_limits<From>::is_signed) {
            if (f < 0 || static_cast<std::make_unsigned_t<From>::t>(f) > std::numeric_limits<To>::max()) {
                return std::nullopt;
            }
        } else {
            if (f > std::numeric_limits<To>::max()) {
                return std::nullopt;
            }
        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        return static_cast<To>(f);
    }
    template<class To, class From>
    requires (std::integral<To> || std::floating_point<To>) && (std::integral<From> || std::floating_point<From>)
    constexpr To CastThrow(From f, const char* err_msg)
    {
        auto r = TryCast<To>(f);
        if (r.has_value())
        {
            return *r;
        }
        throw LuaException{err_msg};
    }
}
