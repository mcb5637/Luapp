#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("Iterate", US, lua::UniqueState, ExtUniqueState)
    {
        US L{};
        CHECK_EQ(0, L.GetTop());

        L.DoStringT("return {[1]=5,[2]=6,[4]=8}");
        CHECK_EQ(1, L.GetTop());
        int i = 0;
        for (auto k : L.Pairs(1))
        {
            CHECK(k == lua::LType::Number);
            CHECK_EQ(L.CheckInt(-2) + 4, L.CheckInt(-1));
            ++i;
        }
        CHECK_EQ(3, i);
        CHECK_EQ(1, L.GetTop());
        i = 0;
        for (auto k : L.IPairs(1))
        {
            CHECK_EQ(lua::Integer{k + 4}, L.CheckInteger(-1));
            ++i;
        }
        CHECK_EQ(2, i);
        CHECK_EQ(1, L.GetTop());
    }
}