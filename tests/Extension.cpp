#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    struct NotLuaCompatible {};
    static_assert(lua::func::detail::Checkable<lua::State, int>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(int, int)>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)()>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, void(*)(int, int)>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, NotLuaCompatible(*)(int, int)>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(int, NotLuaCompatible)>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, std::pair<int, int>(*)()>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, std::tuple<int, int, double>(*)()>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, std::tuple<int, NotLuaCompatible, double>(*)()>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible, int)>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible*, int), 1>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible*, int), 2>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible*, int*), 2>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible*, int), 0, NotLuaCompatible>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible&, int), 0, NotLuaCompatible>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(const NotLuaCompatible*, int), 0, NotLuaCompatible>);
    static_assert(lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(const NotLuaCompatible&, int), 0, NotLuaCompatible>);
    static_assert(!lua::func::detail::AutoTranslateEnabled<lua::State, int(*)(NotLuaCompatible, int), 0, NotLuaCompatible>);

    int ExtFunc(ExtState L)
    {
        auto v = L.CheckVec(1);
        L.Push(v.x + v.y);
        return 1;
    }
    Vec ExtFuncAPI(Vec v)
    {
        return {v.y, v.x};
    }
    TEST_CASE("Extension")
    {
        ExtUniqueState L{};
        CHECK_EQ(0, L.GetTop());
        L.Push<ExtFunc>();

        L.Push(Vec{4.2, 42.0});
        CHECK(L.GetTop() == 2);
        L.GetTableRaw(2, "X");
        CHECK(L.CheckNumber(3) == 4.2);
        L.Pop(1);
        L.GetTableRaw(2, "Y");
        CHECK(L.CheckNumber(3) == 42.0);
        L.Pop(1);

        CHECK(L.CheckVec(-1) == Vec{4.2, 42.0});
        CHECK(L.Check<Vec>(-1) == Vec{4.2, 42.0});
        L.TCall(1, 1);
        CHECK(L.GetTop() == 1);
        CHECK(L.CheckNumber(1) == 4.2 + 42.0);
        L.SetTop(0);

        L.Push<&ExtFuncAPI>();
        auto r = L.TCall<Vec>(Vec{1.0, 5.0});
        CHECK(r == Vec{5.0, 1.0});
        CHECK(L.GetTop() == 1);
    }
}