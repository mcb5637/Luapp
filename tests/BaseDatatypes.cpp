#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("BaseDatatypes", S, lua::UniqueState, ExtUniqueState)
    {
        S L{};
        CHECK(0 == L.GetTop());

        CHECK(L.IsNone(1));
        CHECK(L.IsNoneOrNil(1));
        CHECK(L.Type(1) == lua::LType::None);

        L.Push();
        CHECK(L.IsNil(1));
        CHECK(L.IsNoneOrNil(1));
        CHECK(L.Type(1) == lua::LType::Nil);
        CHECK(std::string_view{"nil"} == L.ConvertToString(1));
        CHECK(std::string{"nil"} == L.ToDebugString(1));
        L.SetTop(0);

        L.Push(1);
        CHECK(L.IsNumber(1));
        if constexpr (S::Capabilities::NativeIntegers)
            CHECK(L.IsInteger(1));

        CHECK(lua::Integer{1} == L.CheckInteger(1));
        CHECK(1.0 == L.template Check<double>(1));
        CHECK(1.0f == L.template Check<float>(1));
        CHECK(L.template Check<int32_t>(1) == 1);
        CHECK(1.0 == L.CheckNumber(1));
        CHECK(L.Type(1) == lua::LType::Number);
        CHECK(std::string_view{"1"} == L.ConvertToString(1));
        CHECK(std::string{"1"} == L.ToDebugString(1));
        L.SetTop(0);

        L.Push(1.3);
        CHECK(L.IsNumber(1));
        if constexpr (S::Capabilities::NativeIntegers)
        {
            CHECK(!L.IsInteger(1));
            CHECK_THROWS_WITH_AS(L.template Check<int32_t>(1), "bad argument #1 (number has no integer representation)", lua::LuaException);
        }
        else
        {
            CHECK(lua::Integer{1} == L.CheckInteger(1));
            CHECK(L.template Check<int32_t>(1) == 1);
        }
        CHECK(1.3 == L.CheckNumber(1));
        CHECK(1.3 == L.template Check<double>(1));
        CHECK(1.3f == L.template Check<float>(1));
        CHECK(L.Type(1) == lua::LType::Number);
        CHECK(std::string_view{"1.3"} == L.ConvertToString(1));
        CHECK(std::string{"1.3"} == L.ToDebugString(1));
        if constexpr (S::Capabilities::NativeIntegers)
            CHECK_THROWS_WITH_AS(L.Push(std::numeric_limits<uint64_t>::max()), "value cannot fit into lua::Integer", lua::LuaException);
        L.Push(std::numeric_limits<double>::max());
        if constexpr (S::Capabilities::NativeIntegers)
            CHECK_THROWS_WITH_AS(L.template Check<int32_t>(-1), "bad argument #-1 (number has no integer representation)", lua::LuaException);
        else
            CHECK_THROWS_WITH_AS(L.template Check<int32_t>(-1), "bad argument #-1 (int expected, value out of range)", lua::LuaException);
        CHECK_THROWS_WITH_AS(L.template Check<float>(-1), "bad argument #-1 (float expected, value out of range)", lua::LuaException);
        if constexpr (S::Capabilities::NativeIntegers)
        {
            L.Push(std::numeric_limits<lua::Integer>::max());
            CHECK_THROWS_WITH_AS(L.template Check<int32_t>(-1), "bad argument #-1 (int expected, value out of range)", lua::LuaException);
        }
        L.SetTop(0);

        L.Push("a");
        CHECK(L.IsString(1));
        CHECK(std::string_view{"a"} == L.CheckString(1));
        CHECK(std::string_view{"a"} == L.CheckStringView(1));
        CHECK(L.Type(1) == lua::LType::String);
        CHECK(std::string_view{"a"} == L.ConvertToString(1));
        CHECK(std::string{"\"a\""} == L.ToDebugString(1));
        L.SetTop(0);

        L.PushFString("a %d", 42);
        CHECK(L.IsString(1));
        CHECK_EQ(std::string_view{"a 42"}, L.CheckString(1));
        CHECK_EQ(std::string_view{"a 42"}, L.template Check<std::string>(1));
        CHECK_EQ(std::string_view{"a 42"}, L.CheckStringView(1));
        CHECK_EQ(std::string_view{"a 42"}, L.template Check<std::string_view>(1));
        CHECK(L.Type(1) == lua::LType::String);
        L.SetTop(0);

        L.Push(true);
        CHECK(L.IsBoolean(1));
        CHECK_EQ(true, L.CheckBool(1));
        CHECK(L.template Check<bool>(1) == true);
        CHECK(L.Type(1) == lua::LType::Boolean);
        CHECK_EQ(std::string_view{"true"}, L.ConvertToString(1));
        CHECK_EQ(std::string{"true"}, L.ToDebugString(1));
        L.SetTop(0);

        L.PushLightUserdata(&L);
        CHECK(L.IsUserdata(1));
        CHECK(L.IsLightUserdata(1));
        CHECK_EQ(static_cast<void*>(&L), L.ToUserdata(1));
        CHECK(L.Type(1) == lua::LType::LightUserdata);
        AssertRegex("userdata: 0x[0-9a-f]+", L.ConvertToString(1));
        AssertRegex("<LightUserdata 0x[0-9a-f]+>", L.ToDebugString(1));
        L.SetTop(0);

        if constexpr (S::Capabilities::Uservalues)
        {
            if constexpr (S::Capabilities::ArbitraryUservalues)
            {
                L.NewUserdata(sizeof(int), 1);
            }
            else
            {
                L.NewUserdata(sizeof(int));
            }
            CHECK(L.IsUserdata(1));
            CHECK(!L.IsLightUserdata(1));
            CHECK(L.Type(1) == lua::LType::Userdata);
            AssertRegex("userdata: 0x[0-9a-f]+", L.ConvertToString(1));
            AssertRegex("<Userdata unknown type 0x[0-9a-f]+>", L.ToDebugString(1));
            L.SetTop(1);

            L.NewTable();
            L.PushValue(2);
            L.SetUserValue(1);
            L.GetUserValue(1);
            CHECK(L.IsTable(3));
            CHECK(L.RawEqual(2, 3));
            L.SetTop(0);
        }

        if constexpr (S::Capabilities::ExternalString)
        {
            L.PushExternalString("xyz");
            CHECK(L.IsString(1));
            CHECK(L.CheckStringView(1) == "xyz");
            L.SetTop(0);

            L.PushExternalString("xyz", 3);
            CHECK(L.IsString(1));
            CHECK(L.CheckStringView(1) == "xyz");
        }
    }

    TEST_CASE_TEMPLATE("cast ints", To, int16_t, uint16_t)
    {
        CHECK(lua::cast_detail::TryCast<To>(int64_t{5}) == To{5});
        CHECK(lua::cast_detail::TryCast<To>(int64_t{0}) == To{0});
        CHECK(lua::cast_detail::TryCast<To>(std::numeric_limits<int64_t>::max()) == std::nullopt);
        CHECK(lua::cast_detail::TryCast<To>(std::numeric_limits<int64_t>::min()) == std::nullopt);
        if constexpr (!std::unsigned_integral<To>)
        {
            CHECK(lua::cast_detail::TryCast<To>(int64_t{-5}) == To{-5});
        }
        else
        {
            CHECK(lua::cast_detail::TryCast<To>(int64_t{-5}) == std::nullopt);
        }

        CHECK(lua::cast_detail::TryCast<To>(uint64_t{5}) == To{5});
        CHECK(lua::cast_detail::TryCast<To>(uint64_t{0}) == To{0});
        CHECK(lua::cast_detail::TryCast<To>(std::numeric_limits<uint64_t>::max()) == std::nullopt);

        CHECK(lua::cast_detail::TryCast<To>(5.0) == To{5});
        CHECK(lua::cast_detail::TryCast<To>(0.0) == To{0});
        CHECK(lua::cast_detail::TryCast<To>(std::numeric_limits<double>::max()) == std::nullopt);
        CHECK(lua::cast_detail::TryCast<To>(-std::numeric_limits<double>::max()) == std::nullopt);
        CHECK(lua::cast_detail::TryCast<To>(std::numeric_limits<double>::min()) == To{0});
        if constexpr (!std::unsigned_integral<To>)
        {
            CHECK(lua::cast_detail::TryCast<To>(-5.0) == To{-5});
            CHECK(lua::cast_detail::TryCast<To>(-std::numeric_limits<double>::min()) == To{0});
        }
        else
        {
            CHECK(lua::cast_detail::TryCast<To>(-5.0) == std::nullopt);
            CHECK(lua::cast_detail::TryCast<To>(-std::numeric_limits<double>::min()) == std::nullopt);
        }
    }
    TEST_CASE("cast float")
    {
        CHECK(lua::cast_detail::TryCast<float>(5.0) == 5.0f);
        CHECK(lua::cast_detail::TryCast<float>(0.0) == 0.0f);
#ifndef _MSC_VER
        CHECK(lua::cast_detail::TryCast<float>(std::numeric_limits<double>::max()) == std::nullopt);
        CHECK(lua::cast_detail::TryCast<float>(-std::numeric_limits<double>::max()) == std::nullopt);
#endif
        CHECK(lua::cast_detail::TryCast<float>(std::numeric_limits<double>::min()) == 0.0f);

        CHECK(lua::cast_detail::TryCast<float>(-5.0) == -5.0f);
        CHECK(lua::cast_detail::TryCast<float>(-std::numeric_limits<double>::min()) == 0.0f);
    }
}