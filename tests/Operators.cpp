#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    TEST_CASE_TEMPLATE("Operators", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        US L{};
        CHECK_EQ(0, L.GetTop());

        L.Push(5);
        L.Push(5);
        L.Push(10);
        CHECK(L.LessThan(1, 3));
        CHECK(L.Compare(1, 3, lua::ComparisonOperator::LessThan));
        CHECK(L.Compare(1, 3, lua::ComparisonOperator::LessThanOrEquals));
        CHECK(L.Equal(1, 2));
        CHECK(L.RawEqual(1, 2));
        CHECK(L.Compare(1, 2, lua::ComparisonOperator::Equals));
        CHECK(L.Compare(1, 2, lua::ComparisonOperator::LessThanOrEquals));

        L.Arithmetic(lua::ArithmeticOperator::Add);
        CHECK_EQ(lua::Integer{15}, L.CheckInteger(2));

        L.Arithmetic(lua::ArithmeticOperator::Subtract);
        CHECK_EQ(lua::Integer{-10}, L.CheckInteger(1));

        L.Push(2);
        L.Arithmetic(lua::ArithmeticOperator::Multiply);
        CHECK_EQ(lua::Integer{-20}, L.CheckInteger(1));

        L.Push(2);
        L.Arithmetic(lua::ArithmeticOperator::Divide);
        CHECK_EQ(lua::Integer{-10}, L.CheckInteger(1));

        L.Arithmetic(lua::ArithmeticOperator::UnaryNegation);
        CHECK_EQ(lua::Integer{10}, L.CheckInteger(1));

        L.Push(4);
        L.Arithmetic(lua::ArithmeticOperator::Modulo);
        CHECK_EQ(lua::Integer{2}, L.CheckInteger(1));

        L.Push(4);
        L.Arithmetic(lua::ArithmeticOperator::Pow);
        CHECK_EQ(lua::Integer{16}, L.CheckInteger(1));
        L.Pop(1);

        if constexpr (S::Capabilities::NativeIntegers)
        {
            L.Push(4);
            L.Push(6);
            L.Arithmetic(S::ArithmeticOperator::BitwiseAnd);
            CHECK_EQ(lua::Integer{4}, L.CheckInteger(1));

            L.Push(2);
            L.Arithmetic(S::ArithmeticOperator::BitwiseOr);
            CHECK_EQ(lua::Integer{6}, L.CheckInteger(1));

            L.Push(2);
            L.Arithmetic(S::ArithmeticOperator::BitwiseXOr);
            CHECK_EQ(lua::Integer{4}, L.CheckInteger(1));

            L.Push(2);
            L.Arithmetic(S::ArithmeticOperator::ShiftLeft);
            CHECK_EQ(lua::Integer{4 << 2}, L.CheckInteger(1));

            L.Push(2);
            L.Arithmetic(S::ArithmeticOperator::ShiftRight);
            CHECK_EQ(lua::Integer{4}, L.CheckInteger(1));

            L.Arithmetic(S::ArithmeticOperator::BitwiseNot);
            CHECK_EQ(~lua::Integer{4}, L.CheckInteger(1));
            L.Pop(1);
        }

        L.Push(16);
        L.Push("abc");
        L.Concat(2);
        CHECK_EQ(std::string_view{"16abc"}, L.CheckString(1));
    }
}