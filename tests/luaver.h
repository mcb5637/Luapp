#pragma once
#include <memory>
#include <regex>

#include <doctest/doctest.h>

#ifdef LUA50
#include <luapp/luapp50.h>
#endif
#ifdef LUA51
#include <luapp/luapp51.h>
#endif
#ifdef LUAJIT
#include <luapp/luappjit.h>
#endif
#ifdef LUA52
#include <luapp/luapp52.h>
#endif
#ifdef LUA53
#include <luapp/luapp53.h>
#endif
#ifdef LUA54
#include <luapp/luapp54.h>
#endif
#ifdef LUA55
#include <luapp/luapp55.h>
#endif

namespace LuappDev
{
    struct Vec
    {
        double x, y;

        auto operator<=>(const Vec&) const = default;
    };
    template<class S>
    struct StateExt
    {
        void Push(Vec f)
        {
            auto* t = static_cast<S*>(this);
            t->NewTable();
            t->Push("X");
            t->Push(f.x);
            t->SetTableRaw(-3);
            t->Push("Y");
            t->Push(f.y);
            t->SetTableRaw(-3);
        }
        Vec CheckVec(int i)
        {
            auto* t = static_cast<S*>(this);
            i = t->ToAbsoluteIndex(i);
            t->CheckType(i, lua::LType::Table);
            t->GetTableRaw(i, "X");
            t->GetTableRaw(i, "Y");
            Vec r{t->CheckNumber(-2), t->CheckNumber(-1)};
            t->Pop(2);
            return r;
        }
        template<class T>
        requires std::same_as<T, Vec>
        Vec Check(int i)
        {
            return CheckVec(i);
        }
    };
    using ExtUniqueState = lua::UniqueState::BindExtensions<StateExt>;
    using ExtState = ExtUniqueState::Base;


    inline bool RegexMatch(const char* p, std::string_view data)
    {
        std::regex pattern{p, std::regex_constants::ECMAScript};
        return std::regex_match(data.begin(), data.end(), pattern);
    }
    inline void AssertRegex(const char* p, std::string_view data)
    {
        CHECK_MESSAGE(RegexMatch(p, data), std::format("expected '{}', found '{}'", p, data));
    }

}
