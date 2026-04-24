#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{

    namespace funcs
    {
        template<class S>
        static int run_foo(S L)
        {
            L.PushValue(1);
            L.Push(42);
            L.Arithmetic(lua::ArithmeticOperator::Add);
            return 1;
        }
        static double run_api(double a, double b)
        {
            return a + b;
        }
    } // namespace funcs

    TEST_CASE_TEMPLATE("RunLua", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        US L{};
        CHECK_EQ(0, L.GetTop());

        L.DoStringT("function fool(a, b) return a+b end");
        CHECK_EQ(0, L.GetTop());
        L.GetGlobal("fool");
        CHECK(L.IsFunction(1));
        CHECK(!L.IsCFunction(1));
        if (L.Version() == 5.0)
            CHECK_EQ(
                std::string{
                    "<function Lua global fool (defined in: [string \"function fool(a, b) return a+b end\"]:1)>"},
                L.ToDebugString(1));
        else
            CHECK_EQ(
                std::string{"<function Lua  fool (defined in: [string \"function fool(a, b) return a+b end\"]:1)>"},
                L.ToDebugString(1));

        L.Push(5);
        L.Push(42);
        L.Push("abc");
        L.TCall(3, 1);
        CHECK_EQ(lua::Integer{5 + 42}, L.CheckInteger(1));
        L.Pop(1);


        std::array toRegister{
            S::FuncReference::template GetRef<&funcs::run_foo<S>>("foo"),
            S::FuncReference::template GetRef<&funcs::run_api>("api1"),
        };

        L.RegisterFuncs(toRegister);
        L.DoStringT("return foo(6, 666)");
        CHECK_EQ(lua::Integer{6 + 42}, L.CheckInteger(1));
        L.SetTop(0);
        L.DoStringT("return api1(5.5, 4.5)");
        CHECK(L.CheckNumber(1) == 10.0);
        L.SetTop(0);

        CHECK(L.DoString("error('number is 6')") != lua::ErrorCode::Success);
        CHECK(L.CheckStringView(1).find("number is 6") != std::string_view::npos);

        L.SetTop(0);
        bool ok = false;
        try
        {
            L.DoStringT("error('number is 6')");
        }
        catch (const lua::LuaException& e)
        {
            if (std::string_view{e.what()}.find("number is 6") != std::string_view::npos)
                ok = true;
        }
        CHECK(ok);

        L.SetTop(0);
        L.GetGlobal("fool");
        CHECK(L.IsFunction(1));
        CHECK(!L.IsCFunction(1));
        auto f = L.Dump();
        CHECK(!f.empty());
        L.SetTop(0);

        CHECK(L.LoadBuffer(f.c_str(), f.length(), nullptr) == lua::ErrorCode::Success);
        L.Push(7);
        L.Push(42);
        L.Push("abc");
        L.TCall(3, 1);
        CHECK_EQ(lua::Integer{7 + 42}, L.CheckInteger(1));
        L.SetTop(0);


        L.DoString("glob = 5; function aglob() return glob; end; return aglob;");
        CHECK(L.IsFunction(1));
        L.PushValue(1);
        L.TCall(0, 1);
        CHECK_EQ(5.0, *L.ToNumber(2));
        L.Pop(1);

        L.GetEnvironment(1);
        L.PushGlobalTable();
        CHECK(L.RawEqual(2, 3));
        L.Pop(2);

        L.NewTable();
        L.Push(42);
        L.SetTableRaw(2, "glob");
        L.SetEnvironment(1);
        L.TCall(0, 1);
        CHECK_EQ(42.0, *L.ToNumber(1));
        L.SetTop(0);


        L.DoString("return function() return 42 end");
        CHECK(L.IsFunction(1));
        if constexpr (S::Capabilities::Environments)
        {
            L.GetEnvironment(1);
            L.PushGlobalTable();
            CHECK(L.RawEqual(2, 3));
            L.Pop(2);
        }
        else
        {
            L.GetEnvironment(1);
            CHECK(L.IsNil(2));
        }
        L.SetTop(0);
    }
}