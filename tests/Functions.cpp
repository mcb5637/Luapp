#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    namespace funcs
    {
        static int foo_raw(lua_State* l)
        {
            lua::State L{l};
            L.PushValue(1);
            L.Push(42);
            L.Arithmetic(lua::ArithmeticOperator::Add);
            return 1;
        }
        template<class S>
        static int run_foo(S L)
        {
            L.PushValue(1);
            L.Push(42);
            L.Arithmetic(lua::ArithmeticOperator::Add);
            return 1;
        }
        template<class S>
        static int foo_up(S L)
        {
            CHECK_EQ(1, L.Debug_GetStackDepth());
            L.PushValue(1);
            L.PushValue(S::Upvalueindex(1));
            L.Arithmetic(lua::ArithmeticOperator::Add);
            return 1;
        }
        template<class S>
        static int ex(S L)
        {
            auto i = L.CheckInt(1);
            throw lua::LuaException{std::format("number is {}", i)};
        }
        struct UnknownExceptionType {};
        template<class S>
        static int exUnknown([[maybe_unused]] S L)
        {
            throw UnknownExceptionType{}; // NOLINT(*-exception-baseclass)
        }
        static double run_api(double a, double b)
        {
            return a + b;
        }
        static double api2()
        {
            return 42.0;
        }
        static void api3(double a)
        {
            CHECK(a == 5.0);
        }
        static std::tuple<double, std::string_view> api4(int i)
        {
            return {static_cast<double>(i) + 0.5, "foo"};
        }
        template<class S>
        static double api5(S L, int i)
        {
            return L.CheckNumber(S::Upvalueindex(1)) + i;
        }
    } // namespace funcs

    std::string ExceptionConverter(std::string_view funcsig)
    {
        try
        {
            throw;
        }
        catch (const funcs::UnknownExceptionType&)
        {
            return std::format("unknown exception type caught: {}", funcsig);
        }
    }

    template<class S>
    struct Close
    {
        bool* closed;

        void C()
        {
            *closed = true;
        }

        static constexpr std::array LuaMetaMethods {
            S::FuncReference::template GetUCRef<&Close::C>(S::MetaEvent::Close),
        };
    };

    TEST_CASE_TEMPLATE("Functions", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        S::GetExConv() = &ExceptionConverter;

        US L{};
        CHECK_EQ(0, L.GetTop());

        L.Push(&funcs::foo_raw);
        CHECK(L.IsFunction(1));
        CHECK(L.IsCFunction(1));
        CHECK(L.Type(1) == lua::LType::Function);

        L.Push(5);
        CHECK_EQ(1, L.TCall(1, 1));
        CHECK_EQ(1, L.GetTop());
        CHECK_EQ(lua::Integer{42 + 5}, L.CheckInteger(1));
        L.SetTop(0);

        L.template Push<funcs::run_foo<S>>();
        CHECK(L.IsFunction(1));
        CHECK(L.IsCFunction(1));
        CHECK(L.Type(1) == lua::LType::Function);

        L.Push(5);
        CHECK_EQ(1, L.TCall(1, 1));
        CHECK_EQ(1, L.GetTop());
        CHECK_EQ(lua::Integer{42 + 5}, L.CheckInteger(1));
        L.SetTop(0);

        L.Push(44);
        L.template Push<funcs::foo_up<S>>(1);
        L.Push(5);
        CHECK_EQ(1, L.TCall(1, L.MULTIRET));
        CHECK_EQ(1, L.GetTop());
        CHECK_EQ(lua::Integer{44 + 5}, L.CheckInteger(1));
        L.SetTop(0);

        L.template Push<funcs::run_foo<S>>();

        L.Push(5);
        L.Push("bar");
        CHECK_EQ(2, L.TCall(2, 2));
        CHECK_EQ(lua::Integer{42 + 5}, L.CheckInteger(1));
        CHECK(L.IsNil(2));
        L.SetTop(0);

        L.template Push<S::template AutoTranslateAPI<funcs::run_api>>();
        L.Push(5.0);
        L.Push(10.05);
        L.TCall(2, 1);
        CHECK(L.template Check<double>(1) == 15.05);
        L.SetTop(0);

        L.template Push<S::template AutoTranslateAPI<funcs::api2>>();
        L.TCall(0, 1);
        CHECK(L.template Check<double>(1) == 42.0);
        L.SetTop(0);

        L.template Push<S::template AutoTranslateAPI<funcs::api3>>();
        L.Push(5.0);
        L.TCall(1, 0);
        L.SetTop(0);

        L.template Push<&funcs::api4>();
        L.Push(5);
        L.TCall(1, 2);
        CHECK(L.template Check<double>(1) == 5.5);
        CHECK(L.template Check<std::string_view>(2) == "foo");
        L.SetTop(0);

        L.Push(5.5);
        L.template Push<&funcs::api5<S>>(1);
        L.Push(500);
        L.TCall(1, 1);
        CHECK(L.template Check<double>(1) == 505.5);
        L.SetTop(0);

        L.template Push<funcs::ex<S>>();
        L.Push(5);
        CHECK(L.PCall(1, 0) == lua::ErrorCode::Runtime);
        CHECK(L.CheckStringView(1).find("number is 5") != std::string_view::npos);
        L.SetTop(0);

        L.template Push<funcs::ex<S>>();
        L.Push(5);
        CHECK_THROWS_WITH_AS(L.TCall(1, 0), doctest::Contains("number is 5"), lua::LuaException);
        L.SetTop(0);

        L.template Push<funcs::exUnknown<S>>();
        CHECK(L.PCall(0, 0) == lua::ErrorCode::Runtime);
        CHECK(L.CheckStringView(1).find("unknown exception type caught:") != std::string_view::npos);
        L.SetTop(0);

        L.SetTop(0);
        struct A
        {
            int i;

            [[nodiscard]] int P(S L) // NOLINT(*-make-member-function-const)
            {
                L.Push(i + L.CheckInt(1) + L.CheckInt(S::Upvalueindex(2)));
                return 1;
            }
            [[nodiscard]] int PC(S L) const
            {
                L.Push(i + 1 + L.CheckInt(1) + L.CheckInt(S::Upvalueindex(2)));
                return 1;
            }
            [[nodiscard]] int G(S L) // NOLINT(*-make-member-function-const)
            {
                L.Push(i);
                return 1;
            }
            [[nodiscard]] int GC(S L) const
            {
                L.Push(i + 1);
                return 1;
            }

            [[nodiscard]] double API(double a) // NOLINT(*-make-member-function-const)
            {
                return static_cast<double>(i) + a;
            }
            [[nodiscard]] double APIC(double a) const
            {
                return static_cast<double>(i) + a + 1.0;
            }
            [[nodiscard]] std::tuple<double, int, std::string_view> PARAMS(double a, std::string_view s, int x) const
            {
                return {static_cast<double>(i) + a, x + i, s};
            }

            double api6(double b, S L, int j)
            {
                return L.CheckNumber(S::Upvalueindex(2)) + j + i + b;
            }
        };
        A a{5};
        L.Push(3);
        L.template Push<&A::P>(a, 1);
        L.Push(4);
        L.TCall(1, 1);
        CHECK_EQ(12, L.CheckInt(1));
        L.SetTop(0);

        L.Push(3);
        L.template Push<&A::PC>(a, 1);
        L.Push(4);
        L.TCall(1, 1);
        CHECK_EQ(13, L.CheckInt(1));
        L.SetTop(0);

        L.template Push<&A::API>(a);
        L.Push(5);
        L.TCall(1, 1);
        CHECK_EQ(10, L.CheckInt(1));
        L.SetTop(0);

        L.template Push<&A::APIC>(a);
        L.Push(5);
        L.TCall(1, 1);
        CHECK_EQ(11, L.CheckInt(1));
        L.SetTop(0);

        L.Push(5.5);
        L.template Push<&A::api6>(a, 1);
        L.Push(6000);
        L.Push(500);
        L.TCall(2, 1);
        CHECK(L.template Check<double>(1) == 6510.5);
        L.SetTop(0);

        std::array toregobj{
            S::FuncReference::template GetRef<&A::G>(a, "G"),
            S::FuncReference::template GetRef<&A::GC>(a, "GC"),
            S::FuncReference::template GetRef<&A::API>(a, "API"),
            S::FuncReference::template GetRef<&A::APIC>(a, "APIC"),
            S::FuncReference::template GetRef<&A::PARAMS>(a, "PARAMS"),
        };
        L.RegisterGlobalLib(toregobj, "AFuncs");
        CHECK_EQ(0, L.GetTop());
        L.GetGlobal("AFuncs");
        CHECK(L.IsTable(1));
        L.GetTableRaw(1, "G");
        CHECK(L.IsFunction(2));
        AssertRegex(R"(<function C  AFuncs\.G \(defined in: C:0x[0-9a-f]+\)>)", L.ToDebugString(2));
        L.TCall(0, 1);
        CHECK(L.IsNumber(2));
        CHECK_EQ(5.0, *L.ToNumber(2));
        L.SetTop(1);
        L.GetTableRaw(1, "GC");
        CHECK(L.IsFunction(2));
        L.TCall(0, 1);
        CHECK(L.IsNumber(2));
        CHECK_EQ(6.0, *L.ToNumber(2));
        L.SetTop(1);
        L.GetTableRaw(1, "API");
        L.Push(1);
        L.TCall(1, 1);
        CHECK(L.CheckNumber(-1) == 6.0);
        L.SetTop(1);
        L.GetTableRaw(1, "APIC");
        L.Push(1);
        L.TCall(1, 1);
        CHECK(L.CheckNumber(-1) == 7.0);
        L.SetTop(1);
        L.GetTableRaw(1, "APIC");
        {
            auto r = L.template TCall<double>(5.0);
            CHECK(r == 11.0);
        }
        CHECK(L.GetTop() == 2);
        L.SetTop(1);
        L.GetTableRaw(1, "PARAMS");
        {
            auto [r, i, s] = L.template TCall<double, int, std::string_view>(5.0, "foo", 1);
            CHECK(r == 10.0);
            CHECK(i == 6);
            CHECK(s == "foo");
        }
        CHECK(L.GetTop() == 4);
        L.SetTop(1);

        L.SetTop(0);
        L.Push(42);
        L.PushLambda([i=0](S l) mutable { l.Push(i++); l.PushValue(S::Upvalueindex(2)); return 2; }, 1);
        L.PushValue(1);
        CHECK(L.template TCall<int, int>() == std::tuple{0, 42});
        L.PushValue(1);
        CHECK(L.template TCall<int, int>() == std::tuple{1, 42});
        L.PushValue(1);
        CHECK(L.template TCall<int, int>() == std::tuple{2, 42});
        CHECK(L.GetMetatable(1) == false);
        L.SetTop(0);

        L.PushLambda([i=0](int x) mutable { return i += x; });
        CHECK(L.Debug_GetUpvalue(1, 1) != nullptr);
        CHECK(L.IsUserdata(2));
        L.Pop(1);
        L.PushValue(1);
        CHECK(L.template TCall<int>(1) == 1);
        L.PushValue(1);
        CHECK(L.template TCall<int>(2) == 3);
        L.PushValue(1);
        CHECK(L.template TCall<int>(3) == 6);
        CHECK(L.GetMetatable(1) == false);
        L.SetTop(0);

        L.PushLambda([i=0](S l) mutable { l.Push(l.CheckInt(1) + i++); return 1; });
        CHECK(L.Debug_GetUpvalue(1, 1) != nullptr);
        CHECK(L.IsUserdata(2));
        L.Pop(1);
        L.PushValue(1);
        CHECK(L.template TCall<int>(1) == 1);
        L.PushValue(1);
        CHECK(L.template TCall<int>(2) == 3);
        L.PushValue(1);
        CHECK(L.template TCall<int>(3) == 5);
        CHECK(L.GetMetatable(1) == false);
        L.SetTop(0);

        L.PushLambda([](S l) { l.Push(l.CheckInt(1) + 5); return 1; });
        CHECK(L.Debug_GetUpvalue(1, 1) == nullptr);
        L.PushValue(1);
        CHECK(L.template TCall<int>(1) == 6);
        L.PushValue(1);
        CHECK(L.template TCall<int>(2) == 7);
        L.PushValue(1);
        CHECK(L.template TCall<int>(3) == 8);
        CHECK(L.GetMetatable(1) == false);
        L.SetTop(0);

        L.PushLambda([](std::optional<int> x) { return x; });
        CHECK(L.Debug_GetUpvalue(1, 1) == nullptr);
        L.PushValue(1);
        CHECK(L.template TCall<int>(1) == 1);
        L.PushValue(1);
        CHECK(L.template TCall<std::optional<int>>() == std::nullopt);
        {
            bool closed = false;
            {
                struct Dtor
                {
                    bool& closed;
                    bool done = false;

                    explicit Dtor(bool& b) : closed(b) {}
                    Dtor(Dtor&& o)  noexcept : closed(o.closed)
                    {
                        o.done = true;
                    }

                    ~Dtor()
                    {
                        if (!done)
                            closed = true;
                    }
                };

                US L2{};
                Dtor dtor{closed};
                L2.PushLambda([x = std::move(dtor)](S) mutable { return 0; });
            }
            CHECK(closed);
        }
        if constexpr (S::Capabilities::CloseSlots)
        {
            L.SetTop(0);
            bool closed = false;
            L.DoStringT(R"(return function(get)
    local toc <close> = get()
    print(tostring(toc))
end)");
            L.PushLambda([c = &closed](S l)
            {
                l.template NewUserClass<Close<S>>(c);
                return 1;
            });
            L.TCall(1, 0);
            CHECK(closed == true);

            closed = false;
            L.PushLambda([](S l)
            {
                l.MarkAsToClose(1);
                return 0;
            });
            L.template NewUserClass<Close<S>>(&closed);
            L.TCall(1, 0);
            CHECK(closed == true);

            closed = false;
            L.PushLambda([](S l)
            {
                l.MarkAsToClose(1);
                l.Pop(1);
                return 0;
            });
            L.template NewUserClass<Close<S>>(&closed);
            L.TCall(1, 0);
            CHECK(closed == true);

            closed = false;
            L.PushLambda([](S l)
            {
                l.MarkAsToClose(1);
                l.CloseSlot(1);
                return 0;
            });
            L.template NewUserClass<Close<S>>(&closed);
            L.TCall(1, 0);
            CHECK(closed == true);
        }
    }
}