#include <doctest/doctest.h>

#include <luaver.h>
#include <luapp/luapp_serialization.h>

namespace LuappDev
{
    template<class S>
    struct SeriUC
    {
        int i = 0;

        static constexpr bool UserClassMetaMethods = true;

        static int Serialize(S L)
        {
            auto* th = L.template CheckUserClass<SeriUC>(1);
            L.Push("SeriUC");
            L.Push(th->i);
            return 2;
        }
        static int Deseri(S L)
        {
            L.template NewUserClass<SeriUC>(L.template Check<int>(1));
            return 1;
        }
    };
    static_assert(lua::userdata::SerializeCpp<lua::State, SeriUC<lua::State>>);
    static_assert(!lua::userdata::DeserializeCpp<lua::State, SeriUC<lua::State>>);

    template<class S>
    struct SeriUCAuto
    {
        int i = 0;

        static constexpr bool UserClassMetaMethods = true;

        static int Serialize(S L)
        {
            auto* th = L.template CheckUserClass<SeriUCAuto<S>>(1);
            L.Push(typename_details::type_name<SeriUCAuto<S>>());
            L.Push(th->i);
            return 2;
        }
        static int Deserialize(S L)
        {
            L.template NewUserClass<SeriUCAuto<S>>(L.template Check<int>(1));
            return 1;
        }
    };
    static_assert(lua::userdata::SerializeCpp<lua::State, SeriUCAuto<lua::State>>);
    static_assert(lua::userdata::DeserializeCpp<lua::State, SeriUCAuto<lua::State>>);

    TEST_CASE_TEMPLATE("Serialization", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;
        SUBCASE("table") {
            std::stringstream val{};

            {
                US L{};

                L.DoStringT("return {5, 'abc', function() return 7 end}");

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                vs.SerializeVariable(-1);
            }
            val.seekp(std::ios_base::beg);
            {
                US L{};

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                vs.DeserializeVariable();
                L.SetGlobal("i");

                CHECK_NOTHROW(L.DoStringT("assert(i[1] == 5)\n"
                    "assert(i[2] == 'abc')\n"
                    "assert(i[3]() == 7)\n"));
            }
        }

        SUBCASE("function globals") {
            std::stringstream val{};

            {
                US L{};

                L.DoStringT(R"(foo = 42
return function()
    return foo + 500
end
)");

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                vs.SerializeVariable(1);
            }
            val.seekp(std::ios_base::beg);
            {
                US L{};

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                vs.DeserializeVariable();
                L.Push(42);
                L.SetGlobal("foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                L.Push(100);
                L.SetGlobal("foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 600);
            }
        }

        SUBCASE("function globals to env") {
            std::stringstream val{};

            {
                US L{};

                L.DoStringT(R"(foo = 42
return function()
    return foo + 500
end
)");

                lua::serialization::LuaSerializer<lua::serialization::StreamIO<std::stringstream>, S, false, true> vs{lua::serialization::StreamIO{val}, S{L}};
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                vs.SerializeVariable(1);
            }
            val.seekp(std::ios_base::beg);
            {
                US L{};

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                vs.DeserializeVariable();
                L.Push(42);
                L.SetGlobal("foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                L.Push(100);
                L.SetGlobal("foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
            }
        }

        SUBCASE("function env") {
            std::stringstream val{};

            {
                US L{};

                L.DoStringT(R"(foo = 1
return function()
    return foo + 500
end, {foo = 42}
)");

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                L.SetEnvironment(1);
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                vs.SerializeVariable(1);
            }
            val.seekp(std::ios_base::beg);
            {
                US L{};

                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                vs.DeserializeVariable();
                L.Push(1);
                L.SetGlobal("foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                L.GetEnvironment(1);
                L.Push(100);
                L.SetTableRaw(-2, "foo");
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 600);
            }
        }

        SUBCASE("function dataonly") {
            std::stringstream val{};

            {
                US L{};

                L.DoStringT(R"(foo = 42
return function()
    return foo + 500
end
)");

                lua::serialization::LuaSerializer<lua::serialization::StreamIO<std::stringstream>, S, true, false> vs{lua::serialization::StreamIO{val}, S{L}};
                L.PushValue(1);
                CHECK(L.template TCall<int>() == 542);
                CHECK_THROWS_WITH_AS(vs.SerializeVariable(1), "functions not allowed", std::format_error);
            }
        }

        if constexpr (S::Capabilities::NativeIntegers)
        {
            SUBCASE("integer")
            {
                US L{};
                std::stringstream str{};
                lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{str}, S{L}};

                L.Push(5);
                CHECK(L.IsInteger(-1));
                vs.SerializeVariable(-1);

                L.SetTop(0);
                str.seekp(std::ios_base::beg);

                vs.DeserializeVariable();
                CHECK(L.IsInteger(-1));
                CHECK(L.template Check<int>(-1) == 5);
            }
        }

        SUBCASE("userclass")
        {
            US L{};
            std::stringstream str{};
            lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{str}, S{L}};

            using UC = SeriUC<S>;
            L.template NewUserClass<UC>(500);
            vs.SerializeVariable(-1);

            str.seekp(std::ios_base::beg);

            vs.GetUserdataDeserializer = [](std::string_view s) -> lua::CFunction
            {
                if (s == "SeriUC")
                    return &S::template CppToCFunction<UC::Deseri>;
                return nullptr;
            };

            CHECK_NOTHROW(vs.DeserializeVariable());
            CHECK(!L.RawEqual(1, 2));
            CHECK(L.template CheckUserClass<UC>(1)->i == L.template CheckUserClass<UC>(2)->i);
        }

        SUBCASE("userclass meta")
        {
            US L{};
            std::stringstream str{};
            lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{str}, S{L}};

            using UC = SeriUCAuto<S>;
            L.template NewUserClass<UC>(500);
            vs.SerializeVariable(-1);

            str.seekp(std::ios_base::beg);

            CHECK_NOTHROW(vs.DeserializeVariable());
            CHECK(!L.RawEqual(1, 2));
            CHECK(L.template CheckUserClass<UC>(1)->i == L.template CheckUserClass<UC>(2)->i);
        }

        SUBCASE("userclass dataonly")
        {
            US L{};
            std::stringstream str{};
            lua::serialization::LuaSerializer<lua::serialization::StreamIO<std::stringstream>, S, true, false> vs{lua::serialization::StreamIO{str}, S{L}};

            using UC = SeriUC<S>;
            L.template NewUserClass<UC>(500);
            CHECK_THROWS_WITH_AS(vs.SerializeVariable(-1), "userdata not allowed", std::format_error);
        }

        SUBCASE("stack")
        {
            US L{};
            std::stringstream str{};
            lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{str}, S{L}};

            L.Push(5);
            L.Push("foo");
            L.NewTable();
            L.PushValue(3);
            vs.SerializeStack();

            L.SetTop(0);
            str.seekp(std::ios_base::beg);

            vs.DeserializeStack();
            CHECK(L.template Check<int>(1) == 5);
            CHECK(L.template Check<std::string_view>(2) == "foo");
            CHECK(L.IsTable(3));
            CHECK(L.RawEqual(3, 4));
        }

        SUBCASE("state")
        {
            {
                std::stringstream val{};
                {
                    US L{};

                    L.DoStringT("function foo() return 42 end\n"
                        "x = {}\n"
                        "x.y = x\n"
                        "x[foo] = 5\n"
                        "x[5] = foo\n"
                        "x[-5.5] = foo\n"
                        "x[x] = foo\n"
                        "y = x\n");

                    L.PushSerializedRegistry();
                    L.Push("bar");
                    L.GetGlobal("x");
                    L.SetTableRaw(-3);
                    L.SetTop(0);


                    lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                    vs.IsGlobalSkipped = [](std::string_view v) { return v == "y"; };
                    vs.SerializeState();
                }
                val.seekp(std::ios_base::beg);
                {
                    US L{};

                    lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                    vs.DeserializeState();

                    CHECK_NOTHROW(L.DoStringT("assert(foo() == 42)\n"
                        "assert(x == x.y)\n"
                        "assert(x[foo] == 5)\n"
                        "assert(x[5] == foo)\n"
                        "assert(x[-5.5] == foo)\n"
                        "assert(x[x] == foo)\n"
                        "assert(y == nil)\n"));

                    L.PushSerializedRegistry();
                    L.Push("bar");
                    L.GetTableRaw(-2);
                    L.GetGlobal("x");
                    CHECK(L.RawEqual(-1, -2));
                }
            }
        }

        if constexpr (S::Capabilities::UpvalueId)
        {
            SUBCASE("upvalue")
            {
                {
                    std::stringstream val{};
                    {
                        US L{};

                        L.DoStringT(R"(function create()
    local x = 0
    return function() return x end, function() x = x + 1 end
end
getx, incx = create()
gety, incy = create()
x2 = getx

assert(x2 == getx)
assert(getx ~= gety)
assert(getx() == 0)
incx()
assert(getx() == 1)
create = nil
)");

                        lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                        vs.SerializeState();
                    }
                    val.seekp(std::ios_base::beg);
                    {
                        US L{};

                        lua::serialization::LuaSerializer vs{lua::serialization::StreamIO{val}, S{L}};
                        vs.DeserializeState();

                        CHECK_NOTHROW(L.DoStringT(R"(assert(create == nil)
assert(x2 == getx)
assert(getx ~= gety)
assert(getx() == 1)
incx()
assert(getx() == 2)
assert(gety() == 0)
incy()
assert(gety() == 1)
)"));
                    }
                }
            }
        }
    }
}