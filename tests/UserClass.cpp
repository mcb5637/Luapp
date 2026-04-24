#include <doctest/doctest.h>

#include <DtorTest.h>
#include <iostream>
#include <luaver.h>

namespace LuappDev
{

    namespace cls
    {
        template<class S>
        class IntHolderOp
        {
            lua::Integer i;

            static int get(S L)
            {
                auto t = L.template CheckUserClass<IntHolderOp>(1);
                L.Push(t->i);
                return 1;
            }
            static int set(S L)
            {
                auto t = L.template CheckUserClass<IntHolderOp>(1);
                t->i = L.CheckInt(2);
                return 0;
            }

        public:
            explicit IntHolderOp(lua::Integer j) : i(j) {}

            static constexpr bool UserClassOperatorTranslate = true;

            auto operator<=>(const IntHolderOp&) const noexcept = default;

            IntHolderOp operator+(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i + o.i}; }
            IntHolderOp operator-(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i - o.i}; }
            IntHolderOp operator*(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i * o.i}; }
            IntHolderOp operator/(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i / o.i}; }
            IntHolderOp operator-() const noexcept { return IntHolderOp{-this->i}; }

            IntHolderOp operator&(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i & o.i}; }
            IntHolderOp operator|(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i | o.i}; }
            IntHolderOp operator^(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i ^ o.i}; }
            IntHolderOp operator~() const noexcept { return IntHolderOp{~this->i}; }
            IntHolderOp operator<<(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i << o.i}; }
            IntHolderOp operator>>(const IntHolderOp& o) const noexcept { return IntHolderOp{this->i >> o.i}; }

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<get>("Get"),
                S::FuncReference::template GetRef<set>("Set"),
            };

            static constexpr std::array LuaMetaMethods{
                S::FuncReference::template GetRef<get>("Get"),
            };
        };

        template<class S>
        class IntHolderLua
        {
            lua::Integer i;

            static int get(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                L.Push(t->i);
                return 1;
            }
            static int set(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                t->i = L.CheckInt(2);
                return 0;
            }

        public:
            explicit IntHolderLua(lua::Integer j) : i(j) {}

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<get>("Get"),
                S::FuncReference::template GetRef<set>("Set"),
            };
            static constexpr bool UserClassMetaMethods = true;

            static int Equals(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.Push(t->i == o->i);
                return 1;
            }
            static int LessThan(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.Push(t->i < o->i);
                return 1;
            }
            static int LessOrEquals(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.Push(t->i <= o->i);
                return 1;
            }
            static int Add(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i + o->i);
                return 1;
            }
            static int Subtract(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i - o->i);
                return 1;
            }
            static int Multiply(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i * o->i);
                return 1;
            }
            static int Divide(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i / o->i);
                return 1;
            }
            static int Pow(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(static_cast<lua::Integer>(std::pow(t->i, o->i)));
                return 1;
            }
            static int UnaryMinus(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                L.template NewUserClass<IntHolderLua>(-t->i);
                return 1;
            }
            static int Concat(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.Push(t->i);
                L.Push(o->i);
                L.Concat(2);
                return 1;
            }
            static int NewIndex(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                if (L.CheckStringView(2) != "i")
                    throw lua::LuaException{"invalid key"};
                t->i = L.CheckInt(3);
                return 0;
            }
            static int Index(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                if (L.CheckStringView(2) != "i")
                    throw lua::LuaException{"invalid key"};
                L.Push(t->i);
                return 1;
            }
            static int Call(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.CheckInt(2);
                L.template NewUserClass<IntHolderLua>(t->i * o);
                return 1;
            }
            static int Modulo(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i % o->i);
                return 1;
            }
            static int Length(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                L.Push(t->i);
                L.ToString(-1);
                auto l = L.RawLength(-1);
                L.Push(static_cast<lua::Integer>(l));
                return 1;
            }
            static int IntegerDivide(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i / o->i + 1);
                return 1;
            }
            static int BitwiseAnd(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i & o->i);
                return 1;
            }
            static int BitwiseOr(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i | o->i);
                return 1;
            }
            static int BitwiseXOr(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i ^ o->i);
                return 1;
            }
            static int BitwiseNot(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                L.template NewUserClass<IntHolderLua>(~t->i);
                return 1;
            }
            static int ShiftLeft(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i << o->i);
                return 1;
            }
            static int ShiftRight(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                auto o = L.template CheckUserClass<IntHolderLua>(2);
                L.template NewUserClass<IntHolderLua>(t->i >> o->i);
                return 1;
            }
            static int ToString(S L)
            {
                auto t = L.template CheckUserClass<IntHolderLua>(1);
                L.Push(std::format("IntHolder {}", t->i));
                return 1;
            }
        };

        template<class S>
        class IntHolderAPI
        {
            lua::Integer i;

            lua::Integer get()
            {
                return i;
            }
            void set(lua::Integer s)
            {
                i = s;
            }

        public:
            explicit IntHolderAPI(lua::Integer j) : i(j) {}

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetUCRef<&IntHolderAPI::get>("Get"),
                S::FuncReference::template GetUCRef<&IntHolderAPI::set>("Set"),
            };
            static constexpr bool UserClassMetaMethods = true;

            bool Equals(IntHolderAPI* o)
            {
                return i == o->i;
            }
            bool LessThan(IntHolderAPI& o)
            {
                return i < o.i;
            }
            bool LessOrEquals(const IntHolderAPI* o)
            {
                return i <= o->i;
            }
            auto Add(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i + o->i);
            }
            auto Subtract(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i - o->i);
            }
            auto Multiply(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i * o->i);
            }
            auto Divide(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i / o->i);
            }
            auto Pow(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, static_cast<lua::Integer>(std::pow(i, o->i)));
            }
            auto UnaryMinus()
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, -i);
            }
            std::string Concat(const IntHolderAPI* o)
            {
                return std::format("{}{}", i, o->i);
            }
            void NewIndex(std::string_view k, lua::Integer v)
            {
                if (k != "i")
                    throw lua::LuaException{"invalid key"};
                i = v;
            }
            lua::Integer Index(std::string_view k)
            {
                if (k != "i")
                    throw lua::LuaException{"invalid key"};
                return i;
            }
            auto Call(lua::Integer o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i * o);
            }
            auto Modulo(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i % o->i);
            }
            auto Length()
            {
                return std::format("{}", i).size();
            }
            auto IntegerDivide(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i / o->i + 1);
            }
            auto BitwiseAnd(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i & o->i);
            }
            auto BitwiseOr(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i | o->i);
            }
            auto BitwiseXOr(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i ^ o->i);
            }
            auto BitwiseNot()
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, ~i);
            }
            auto ShiftLeft(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i << o->i);
            }
            auto ShiftRight(const IntHolderAPI* o)
            {
                return lua::PushNewUserClass(std::in_place_type<IntHolderAPI>, i >> o->i);
            }
            auto ToString()
            {
                return std::format("IntHolder {}", i);
            }
        };

        template<class S>
        class InheritanceTestB
        {
        protected:
            lua::Integer i;

            static int geti(S L)
            {
                auto t = L.template CheckUserClass<InheritanceTestB>(1);
                L.Push(t->i);
                return 1;
            }
            static int seti(S L)
            {
                auto t = L.template CheckUserClass<InheritanceTestB>(1);
                t->i = L.CheckInt(2);
                return 0;
            }

        public:
            explicit InheritanceTestB(lua::Integer j) : i(j) {}
            virtual ~InheritanceTestB() = default;

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<geti>("GetI"),
                S::FuncReference::template GetRef<seti>("SetI"),
            };

            InheritanceTestB operator+(const InheritanceTestB& o) const { return InheritanceTestB{i + o.i}; }
            InheritanceTestB operator-() const { return InheritanceTestB{-i}; }
        };
        template<class S>
        class InheritanceTestD : public InheritanceTestB<S>
        {
            lua::Number j;
            static int getj(S L)
            {
                auto t = L.template CheckUserClass<InheritanceTestD>(1);
                L.Push(t->j);
                return 1;
            }
            static int setj(S L)
            {
                auto t = L.template CheckUserClass<InheritanceTestD>(1);
                t->j = L.CheckNumber(2);
                return 0;
            }

        public:
            using InheritsFrom = std::tuple<InheritanceTestB<S>>;

            InheritanceTestD(lua::Integer i, lua::Number n) : InheritanceTestB<S>(i), j(n) {}

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<getj>("GetJ"),
                S::FuncReference::template GetRef<setj>("SetJ"),
            };

            InheritanceTestD operator+(const InheritanceTestD& o) const { return {InheritanceTestB<S>::i + o.i, j + o.j}; }
        };
        template<class S>
        class InheritanceTestF : public InheritanceTestB<S>
        {
            std::function<void()> f;

        public:
            using InheritsFrom = std::tuple<InheritanceTestB<S>>;

            static constexpr std::array<typename S::FuncReference, 0> LuaMethods{};

            explicit InheritanceTestF(std::function<void()> v) : InheritanceTestB<S>(42), f(std::move(v)) {}

            ~InheritanceTestF() override { f(); }
        };

        template<class S>
        class Accumulate
        {
        protected:
            lua::Number n = 0.0;

        private:
            int Add(S L)
            {
                n += L.CheckNumber(2);
                return 0;
            }
            [[nodiscard]] int Get(S L) const
            {
                L.Push(n);
                return 1;
            }
            // ReSharper disable once CppMemberFunctionMayBeStatic
            int Virt(S L)
            {
                L.Push(5);
                return 1;
            }
        public:

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetUCRef<&Accumulate::Add>("Add"),
                S::FuncReference::template GetUCRef<&Accumulate::Get>("Get"),
                S::FuncReference::template GetUCRef<&Accumulate::Virt>("Virt"),
            };
        };
        template<class S>
        class Append
        {
        protected:
            std::string s;

        private:
            int AddS(S L)
            {
                s += L.CheckStringView(2);
                return 0;
            }
            [[nodiscard]] int GetS(S L) const
            {
                L.Push(s);
                return 1;
            }
        public:

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetUCRef<&Append::AddS>("AddS"),
                S::FuncReference::template GetUCRef<&Append::GetS>("GetS"),
            };
        };

        template<class S>
        class MultiInheritance : public Accumulate<S>, public Append<S>
        {
            std::function<void()> f;
            static int Virt(S L)
            {
                [[maybe_unused]] auto* th = L.template CheckUserClass<MultiInheritance>(1);
                L.Push(10);
                return 1;
            }

            [[nodiscard]] std::string Format(int a, std::string_view c, const MultiInheritance& r)
            {
                return std::format("a={}, c={}, n={}, s={}", a, c, this->n, r.s);
            }
            [[nodiscard]] lua::PushNewUserClass<MultiInheritance, std::function<void()>> Clone() const
            {
                return lua::userdata::PushNewUserClass{std::in_place_type<MultiInheritance>, this->f};
            }
            static_assert(lua::func::detail::AutoTranslateEnabled<S, decltype(&MultiInheritance::Format), 0, MultiInheritance>);
            static_assert(lua::func::detail::IsUserClass<MultiInheritance*, MultiInheritance> && std::is_pointer_v<MultiInheritance*>);

        public:
            explicit MultiInheritance(std::function<void()> v) : f(std::move(v)) {}
            ~MultiInheritance()
            {
                f();
            }

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<Virt>("Virt"),
                S::FuncReference::template GetUCRef<&MultiInheritance::Format>("Format"),
                S::FuncReference::template GetUCRef<&MultiInheritance::Clone>("Clone"),
                S::FuncReference::template GetRef<Virt>(S::MetaEvent::Add),
                S::FuncReference::template GetUCRef<&MultiInheritance::Format>(S::MetaEvent::Subtract),
            };

            using InheritsFrom = std::tuple<Accumulate<S>, Append<S>>;

            static lua::PushNewUserClass<MultiInheritance, std::function<void()>> SClone(lua::UserClassChecked<MultiInheritance> th)
            {
                return lua::PushNewUserClass{std::in_place_type<MultiInheritance>, th->f};
            }
        };

        template<class S>
        class UservalueTest
        {
            static int get(S L)
            {
                L.template CheckUserClass<UservalueTest>(1);
                if constexpr (S::Capabilities::ArbitraryUservalues)
                {
                    for (int i = 0; i < NumberUserValues; ++i)
                        L.GetUserValue(1, i + 1);
                }
                else if constexpr (S::Capabilities::Uservalues)
                {
                    L.GetUserValue(1);
                }
                return NumberUserValues;
            }
            static int set(S L)
            {
                L.template CheckUserClass<UservalueTest>(1);
                if constexpr (S::Capabilities::ArbitraryUservalues)
                {
                    for (int i = 0; i < NumberUserValues; ++i)
                        L.SetUserValue(1, i + 1);
                }
                else if constexpr (S::Capabilities::Uservalues)
                {
                    L.SetUserValue(1);
                }
                return 0;
            }
            static int n(S L)
            {
                L.Push(NumberUserValues);
                return 1;
            }

        public:
            static constexpr int NumberUserValues =
                S::Capabilities::Uservalues ? (S::Capabilities::ArbitraryUservalues ? 3 : 1) : 0;

            static constexpr std::array LuaMethods{
                S::FuncReference::template GetRef<get>("Get"),
                S::FuncReference::template GetRef<set>("Set"),
                S::FuncReference::template GetRef<n>("n"),
            };
        };
    }

    template<class IntHolderLua, class S>
    void do_holder_lua_tests(S L)
    {

        L.template NewUserClass<IntHolderLua>(5);
        L.SetGlobal("i");
        L.template NewUserClass<IntHolderLua>(7);
        CHECK_EQ(std::string_view{"IntHolder 7"}, L.ConvertToString(1));
        L.Pop(1);
        L.SetGlobal("j");

        L.DoStringT("assert(i:Get()==5);\n"
                    "j:Set(10);\n"
                    "assert(j:Get()==10);\n"
                    "assert((-i):Get()==-5);\n"
                    "assert((i+j):Get()==15);\n"
                    "assert((i-j):Get()==-5);\n"
                    "assert((i*j):Get()==50);\n"
                    "assert((j/i):Get()==2);\n"
                    "assert(i<=j);\n"
                    "assert(i<j);\n"
                    "assert(i~=j);\n"
                    "assert(j>i);\n"
                    "assert(j>=i);\n"
                    "assert(i==i);\n");
        L.DoStringT("assert((i^j):Get()==5^10);\n"
                    "assert(i.i==5);\n"
                    "local k=i+j; k.i=3; assert(k:Get()==3);\n"
                    "assert((i..j)=='510');\n"
                    "assert(i(6).i==5*6);\n");
        if constexpr (S::Capabilities::MetatableLengthModulo)
        {
            L.DoStringT("local a = i+j; a.i=4;\n"
                        "assert((j%a).i==2);\n"
                        "assert(#j==2)\n");
        }
        if constexpr (S::Capabilities::NativeIntegers)
        {
            L.DoStringT("assert((j//i):Get()==3);\n"
                        "j:Set(4); i:Set(2); local k=i+j; k:Set(6);\n"
                        "assert((i&k):Get()==2);\n"
                        "assert((i|j):Get()==6);\n"
                        "assert((i~k):Get()==4);\n"
                        "assert((~i):Get()==~2);\n"
                        "assert((j<<i):Get()==16);\n"
                        "assert((j>>i):Get()==1);\n");
        }

    }

    TEST_CASE_TEMPLATE("UserClass", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        using IntHolderOp = cls::IntHolderOp<S>;
        using IntHolderLua = cls::IntHolderLua<S>;
        using IntHolderAPI = cls::IntHolderAPI<S>;
        using InheritanceTestB = cls::InheritanceTestB<S>;
        using InheritanceTestD = cls::InheritanceTestD<S>;
        using InheritanceTestF = cls::InheritanceTestF<S>;
        using MultiInheritance = cls::MultiInheritance<S>;

        static_assert(lua::userdata::HasLuaMethods<IntHolderOp>);
        static_assert(lua::userdata::HasLuaMetaMethods<IntHolderOp>);
        static_assert(!lua::userdata::EquatableCpp<S, IntHolderOp>);
        static_assert(lua::userdata::EquatableOp<IntHolderOp>);
        static_assert(!lua::userdata::LessThanCpp<S, IntHolderOp>);
        static_assert(lua::userdata::LessThanOp<IntHolderOp>);
        static_assert(!lua::userdata::LessThanEqualsCpp<S, IntHolderOp>);
        static_assert(lua::userdata::LessThanEqualsOp<IntHolderOp>);
        static_assert(!lua::userdata::AddCpp<S, IntHolderOp>);
        static_assert(lua::userdata::AddOp<IntHolderOp>);
        static_assert(!lua::userdata::SubtractCpp<S, IntHolderOp>);
        static_assert(lua::userdata::SubtractOp<IntHolderOp>);
        static_assert(!lua::userdata::MultiplyCpp<S, IntHolderOp>);
        static_assert(lua::userdata::MultiplyOp<IntHolderOp>);
        static_assert(!lua::userdata::DivideCpp<S, IntHolderOp>);
        static_assert(lua::userdata::DivideOp<IntHolderOp>);
        static_assert(!lua::userdata::IntegerDivideCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::ModuloCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::PowCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::UnaryMinusCpp<S, IntHolderOp>);
        static_assert(lua::userdata::UnaryMinusOp<IntHolderOp>);
        static_assert(!lua::userdata::BitwiseAndCpp<S, IntHolderOp>);
        static_assert(lua::userdata::BitwiseAndOp<IntHolderOp>);
        static_assert(!lua::userdata::BitwiseOrCpp<S, IntHolderOp>);
        static_assert(lua::userdata::BitwiseOrOp<IntHolderOp>);
        static_assert(!lua::userdata::BitwiseXOrCpp<S, IntHolderOp>);
        static_assert(lua::userdata::BitwiseXOrOp<IntHolderOp>);
        static_assert(!lua::userdata::BitwiseNotCpp<S, IntHolderOp>);
        static_assert(lua::userdata::BitwiseNotOp<IntHolderOp>);
        static_assert(!lua::userdata::ShiftLeftCpp<S, IntHolderOp>);
        static_assert(lua::userdata::ShiftLeftOp<IntHolderOp>);
        static_assert(!lua::userdata::ShiftRightCpp<S, IntHolderOp>);
        static_assert(lua::userdata::ShiftRightOp<IntHolderOp>);
        static_assert(!lua::userdata::LengthCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::ConcatCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::NewIndexCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::CallCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::IndexCpp<S, IntHolderOp>);
        static_assert(!lua::userdata::ToStringCpp<S, IntHolderOp>);


        static_assert(lua::userdata::HasLuaMethods<IntHolderLua>);
        static_assert(!lua::userdata::HasLuaMetaMethods<IntHolderLua>);
        static_assert(lua::userdata::EquatableCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::EquatableOp<IntHolderLua>);
        static_assert(lua::userdata::LessThanCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::LessThanOp<IntHolderLua>);
        static_assert(lua::userdata::LessThanEqualsCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::LessThanEqualsOp<IntHolderLua>);
        static_assert(lua::userdata::AddCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::AddOp<IntHolderLua>);
        static_assert(lua::userdata::SubtractCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::SubtractOp<IntHolderLua>);
        static_assert(lua::userdata::MultiplyCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::MultiplyOp<IntHolderLua>);
        static_assert(lua::userdata::DivideCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::DivideOp<IntHolderLua>);
        static_assert(lua::userdata::IntegerDivideCpp<S, IntHolderLua>);
        static_assert(lua::userdata::ModuloCpp<S, IntHolderLua>);
        static_assert(lua::userdata::PowCpp<S, IntHolderLua>);
        static_assert(lua::userdata::UnaryMinusCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::UnaryMinusOp<IntHolderLua>);
        static_assert(lua::userdata::BitwiseAndCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::BitwiseAndOp<IntHolderLua>);
        static_assert(lua::userdata::BitwiseOrCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::BitwiseOrOp<IntHolderLua>);
        static_assert(lua::userdata::BitwiseXOrCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::BitwiseXOrOp<IntHolderLua>);
        static_assert(lua::userdata::BitwiseNotCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::BitwiseNotOp<IntHolderLua>);
        static_assert(lua::userdata::ShiftLeftCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::ShiftLeftOp<IntHolderLua>);
        static_assert(lua::userdata::ShiftRightCpp<S, IntHolderLua>);
        static_assert(!lua::userdata::ShiftRightOp<IntHolderLua>);
        static_assert(lua::userdata::LengthCpp<S, IntHolderLua>);
        static_assert(lua::userdata::ConcatCpp<S, IntHolderLua>);
        static_assert(lua::userdata::NewIndexCpp<S, IntHolderLua>);
        static_assert(lua::userdata::CallCpp<S, IntHolderLua>);
        static_assert(lua::userdata::IndexCpp<S, IntHolderLua>);
        static_assert(lua::userdata::ToStringCpp<S, IntHolderLua>);

        static_assert(lua::userdata::HasLuaMethods<IntHolderAPI>);
        static_assert(!lua::userdata::HasLuaMetaMethods<IntHolderAPI>);
        static_assert(lua::userdata::EquatableCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::EquatableOp<IntHolderAPI>);
        static_assert(lua::userdata::LessThanCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::LessThanOp<IntHolderAPI>);
        static_assert(lua::userdata::LessThanEqualsCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::LessThanEqualsOp<IntHolderAPI>);
        static_assert(lua::userdata::AddCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::AddOp<IntHolderAPI>);
        static_assert(lua::userdata::SubtractCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::SubtractOp<IntHolderAPI>);
        static_assert(lua::userdata::MultiplyCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::MultiplyOp<IntHolderAPI>);
        static_assert(lua::userdata::DivideCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::DivideOp<IntHolderAPI>);
        static_assert(lua::userdata::IntegerDivideCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::ModuloCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::PowCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::UnaryMinusCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::UnaryMinusOp<IntHolderAPI>);
        static_assert(lua::userdata::BitwiseAndCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::BitwiseAndOp<IntHolderAPI>);
        static_assert(lua::userdata::BitwiseOrCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::BitwiseOrOp<IntHolderAPI>);
        static_assert(lua::userdata::BitwiseXOrCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::BitwiseXOrOp<IntHolderAPI>);
        static_assert(lua::userdata::BitwiseNotCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::BitwiseNotOp<IntHolderAPI>);
        static_assert(lua::userdata::ShiftLeftCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::ShiftLeftOp<IntHolderAPI>);
        static_assert(lua::userdata::ShiftRightCpp<S, IntHolderAPI>);
        static_assert(!lua::userdata::ShiftRightOp<IntHolderAPI>);
        static_assert(lua::userdata::LengthCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::ConcatCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::NewIndexCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::CallCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::IndexCpp<S, IntHolderAPI>);
        static_assert(lua::userdata::ToStringCpp<S, IntHolderAPI>);

        static_assert(!lua::userdata::InheritsDefined<IntHolderLua>);
        static_assert(lua::userdata::InheritsDefined<InheritanceTestD>);
        static_assert(lua::userdata::InheritsDefined<InheritanceTestF>);

        US L{};
        CHECK_EQ(0, L.GetTop());

        L.template NewUserClass<IntHolderOp>(5);
        L.SetGlobal("i");
        L.template NewUserClass<IntHolderOp>(7);
        CHECK(L.template OptionalUserClass<IntHolderLua>(-1) == nullptr);
        AssertRegex("(class )?LuappDev::cls::IntHolderOp<(class )?lua::decorator::State<[:, [:alnum:]]+> ?>: 0x[0-9a-f]+", L.ConvertToString(1));
        AssertRegex("<Userdata (class )?LuappDev::cls::IntHolderOp<(class )?lua::decorator::State<[:, [:alnum:]]+> ?> 0x[0-9a-f]+>", L.ToDebugString(1));
        CHECK(L.GetMetaField(1, "Get"));
        CHECK(L.IsCFunction(-1));
        L.SetTop(1);
        L.SetGlobal("j");

        L.DoStringT("assert(i:Get()==5);\n"
                    "j:Set(10);\n"
                    "assert(j:Get()==10);\n"
                    "assert((-i):Get()==-5);\n"
                    "assert((i+j):Get()==15);\n"
                    "assert((i-j):Get()==-5);\n"
                    "assert((i*j):Get()==50);\n"
                    "assert((j/i):Get()==2);\n"
                    "assert(i<=j);\n"
                    "assert(i<j);\n"
                    "assert(i~=j);\n"
                    "assert(j>i);\n"
                    "assert(j>=i);\n"
                    "assert(i==i);\n");
        if constexpr (S::Capabilities::NativeIntegers)
        {
            L.DoStringT("j:Set(4); i:Set(2); local k=i+j; k:Set(6);\n"
                        "assert((i&k):Get()==2);\n"
                        "assert((i|j):Get()==6);\n"
                        "assert((i~k):Get()==4);\n"
                        "assert((~i):Get()==~2);\n"
                        "assert((j<<i):Get()==16);\n"
                        "assert((j>>i):Get()==1);\n");
        }

        do_holder_lua_tests<IntHolderLua, S>(L);
        do_holder_lua_tests<IntHolderAPI, S>(L);

        L.template NewUserClass<InheritanceTestD>(5, 10.0);
        CHECK_NOTHROW(L.template CheckUserClass<InheritanceTestB>(-1));
        CHECK_NOTHROW(L.template CheckUserClass<InheritanceTestD>(-1));
        CHECK(L.template OptionalUserClass<InheritanceTestF>(-1) == nullptr);
        CHECK(L.template OptionalUserClass<IntHolderLua>(-1) == nullptr);
        L.SetGlobal("inhd");

        CHECK_NOTHROW(L.template NewUserClass<InheritanceTestB>(5));
        CHECK_NOTHROW(L.template CheckUserClass<InheritanceTestB>(-1));
        CHECK(L.template OptionalUserClass<InheritanceTestF>(-1) == nullptr);
        CHECK(L.template OptionalUserClass<InheritanceTestD>(-1) == nullptr);
        CHECK(L.template OptionalUserClass<IntHolderLua>(-1) == nullptr);
        L.SetGlobal("inhb");

        CHECK_NOTHROW(L.DoStringT("assert(inhd:GetI()==5);\n"
                    "assert(inhd:GetJ()==10);\n"
                    "assert(inhb:GetI()==5);\n"
                    "assert(inhb.GetJ == nil);\n"));

        if constexpr (S::Capabilities::Uservalues)
        {
            L.template NewUserClass<cls::UservalueTest<S>>();
            L.SetGlobal("uvt");
            CHECK_NOTHROW(L.DoStringT("if uvt:n()==3 then\n"
                        "uvt:Set(1,2,3)\n"
                        "local a,b,c = uvt:Get()\n"
                        "assert(a==3)\n"
                        "assert(b==2)\n"
                        "assert(c==1)\n"
                        "elseif uvt:n()==1 then\n"
                        "uvt:Set({1})\n"
                        "assert(uvt:Get()[1]==1)\n"
                        "end"));
        }

        bool closed = false;
        {
            US L2{};
            L2.template NewUserClass<cls::DtorTest>([&closed]() { closed = true; });
            // closing the state forces everything to get gcd
        }
        CHECK(closed);
        closed = false;
        {
            US L2{};
            L2.template NewUserClass<InheritanceTestF>([&closed]() { closed = true; });
            CHECK_NOTHROW(L2.template CheckUserClass<InheritanceTestF>(-1));
            CHECK_NOTHROW(L2.template CheckUserClass<InheritanceTestB>(-1));
            CHECK(L2.template OptionalUserClass<InheritanceTestD>(-1) == nullptr);
            CHECK(L2.template OptionalUserClass<IntHolderLua>(-1) == nullptr);
            // closing the state forces everything to get gcd
        }
        CHECK(closed);

        int num_closed = 0;
        {
            US L2{};
            L2.template NewUserClass<MultiInheritance>([&num_closed]() { ++num_closed; });
            CHECK_NOTHROW(L2.template CheckUserClass<MultiInheritance>(-1));
            L2.SetGlobal("i");
            L2.template Push<MultiInheritance::SClone>();
            L2.SetGlobal("Clone");
            CHECK_NOTHROW(L2.DoStringT("j = i:Clone()\n"
                        "assert(i:Get() == 0)\n"
                        "i:Add(42)\n"
                        "assert(i:Get() == 42)\n"
                        "assert(i:GetS() == '')\n"
                        "i:AddS('foo')\n"
                        "assert(i:GetS() == 'foo')\n"
                        "assert(i:Virt() == 10)\n"
                        "assert(i:Format(5, 'xy', i) == 'a=5, c=xy, n=42, s=foo')\n"
                        "assert(j:Get() == 0)\n"
                        "j:Add(42)\n"
                        "assert(j:Get() == 42)\n"
                        "k = Clone(j)\n"
                        "assert(k:Get() == 0)\n"
                        "k:Add(42)\n"
                        "assert(k:Get() == 42)\n"
                        "assert(k['__add'])\n"
                        "assert(k['__sub'])\n"));
        }
        CHECK(num_closed == 3);
    }
}