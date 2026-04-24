#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("Tables", S, lua::UniqueState, ExtUniqueState)
    {
        S L{};
        CHECK_EQ(0, L.GetTop());

        L.NewTable();
        CHECK_EQ(1, L.GetTop());
        CHECK(L.IsTable(1));
        AssertRegex("<table 0x[0-9a-f]+>", L.ToDebugString(1));

        L.Push("a");
        L.Push(42);
        L.SetTable(1);
        CHECK_EQ(1, L.GetTop());
        L.Push("a");
        L.GetTable(1);
        CHECK_EQ(lua::Integer{42}, L.CheckInteger(2));
        L.Pop(1);
        CHECK_EQ(1, L.GetTop());

        L.Push("a");
        L.Push(43);
        L.SetTableRaw(1);
        CHECK_EQ(1, L.GetTop());
        L.Push("a");
        L.GetTableRaw(1);
        CHECK_EQ(lua::Integer{43}, L.CheckInteger(2));
        L.Pop(1);
        CHECK_EQ(1, L.GetTop());

        L.Push(44);
        L.SetTableRaw(1, "a");
        CHECK_EQ(1, L.GetTop());
        L.GetTableRaw(1, "a");
        CHECK_EQ(lua::Integer{44}, L.CheckInteger(2));
        L.Pop(1);
        CHECK_EQ(1, L.GetTop());

        L.Push("a");
        L.SetTableRaw(1, 1);
        L.GetTableRaw(1, 1);
        CHECK_EQ(std::string_view{"a"}, L.CheckString(2));
        L.Pop(1);
        CHECK_EQ(1, L.GetTop());

        CHECK(!L.GetMetatable(1));
        L.NewTable();
        L.PushValue(2);
        CHECK(L.SetMetatable(1));
        CHECK(L.GetMetatable(1));
        CHECK_EQ(L.ToPointer(2), L.ToPointer(3));
        CHECK(L.RawEqual(2, 3));
        L.SetTop(0);

        L.Push("abc");
        L.SetGlobal("xyz");
        CHECK_EQ(0, L.GetTop());
        L.GetGlobal("xyz");
        CHECK_EQ(1, L.GetTop());
        CHECK_EQ(std::string_view{"abc"}, L.ToString(1));
        L.SetTop(0);

        L.GetSubTable("glob");
        CHECK_EQ(1, L.GetTop());
        CHECK(L.IsTable(1));
        L.GetSubTable("foo", 1);
        CHECK_EQ(2, L.GetTop());
        CHECK(L.IsTable(2));
        L.GetTableRaw(1, "foo");
        CHECK(L.RawEqual(2, 3));
        L.Pop(2);
        L.GetGlobal("glob");
        CHECK(L.RawEqual(1, 2));
        L.SetTop(0);

        L.DoStringT("local t = {5,6,7}; setmetatable(t, {__len=function() return 5; end}); return t;");
        CHECK_EQ(3u, L.RawLength(1));

        if constexpr (S::Capabilities::MetatableLengthModulo)
        {
            L.ObjLength(1);
            CHECK_EQ(S::Capabilities::MetatableLengthOnTables ? 5 : 3, L.CheckInt(2));
        }
        L.SetTop(0);

        L.DoStringT("local t = {}; t.t = {[1]=t}; return t;");
        AssertRegex("\\{\n\tt = \\{\n\t\t\\[1\\] = <table, recursion 0x[0-9a-f]+>,\n\t\\},\n\\}",
                    L.ToDebugString(1, 10));
    }
}