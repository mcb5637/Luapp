#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("StackAccess", S, lua::UniqueState, ExtUniqueState)
    {
        S L{};

        CHECK(0 == L.GetTop());

        L.Push(1);
        L.Push(true);
        CHECK(2 == L.GetTop());
        CHECK(L.IsNumber(1));
        CHECK(L.IsBoolean(2));

        L.PushValue(2);
        CHECK(3 == L.GetTop());
        CHECK(L.IsBoolean(3));
        L.SetTop(2);
        CHECK(2 == L.GetTop());
        CHECK(2 == L.ToAbsoluteIndex(-1));
        CHECK(1 == L.ToAbsoluteIndex(-2));
        CHECK(L.REGISTRYINDEX == L.ToAbsoluteIndex(L.REGISTRYINDEX));

        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(L.CheckBool(2));

        L.Push();
        L.Insert(2);
        CHECK(3 == L.GetTop());
        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(L.IsNil(2));
        CHECK(L.CheckBool(3));

        L.Push("");
        L.Replace(2);
        CHECK(3 == L.GetTop());
        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(std::string_view("") == L.CheckString(2));
        CHECK(L.CheckBool(3));

        L.Copy(1, 2);
        CHECK(3 == L.GetTop());
        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(lua::Integer{1} == L.CheckInteger(2));
        CHECK(L.CheckBool(3));

        L.Remove(2);
        CHECK(2 == L.GetTop());
        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(L.CheckBool(2));

        L.Pop(2);
        CHECK(0 == L.GetTop());
    }
}