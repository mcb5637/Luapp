#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE("AutoCleanStack")
    {
        lua::UniqueState L{};

        CHECK(L.GetTop() == 0);
        {
            auto a = L.AutoCleanStack();
            L.Push();
            L.Push(5);
            CHECK(L.GetTop() == 2);
        }
        CHECK(L.GetTop() == 0);
    }
}