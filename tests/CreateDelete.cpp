#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <luaver.h>
#include <DtorTest.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("CreateDelete", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        S L{};

        US closer{L};

        CHECK(L.GetState() == closer.GetState());

        bool closed = false;
        {
            closer.template NewUserClass<cls::DtorTest>([&closed]() { closed = true; });

            CHECK(!closed);

            US c2{std::move(closer)};
            CHECK(L.GetState() == c2.GetState());
            CHECK(closer.GetState() == static_cast<lua_State*>(nullptr));

            CHECK(!closed);
        }

        CHECK(closed);
    }
}