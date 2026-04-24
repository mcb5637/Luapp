#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    template<class S>
    static int black_magic(S L)
    {
        lua::DebugInfo i{};
        CHECK_EQ(3, L.Debug_GetStackDepth()); // black_magic, foo, main chunk
        CHECK(L.Debug_GetStack(1, i, lua::DebugInfoOptions::Name, false));
        CHECK_EQ(std::string_view{"foo"}, i.Name);
        CHECK_EQ(std::string_view{"foo"}, std::string_view{L.Debug_GetNameForStackFunc(1)});
        int num = 0;
        for (const auto& l : L.Debug_Locals(1))
        {
            if (l.Name == std::string_view{"l"})
            {
                CHECK_EQ(lua::Integer{1}, L.CheckInteger(-1));
                L.Push(42);
                CHECK_EQ(std::string_view{"l"}, L.Debug_SetLocal(1, l.LocalNum));
                ++num;
            }
            else if (l.Name == std::string{"x"})
            {
                CHECK_EQ(lua::Integer{5}, L.CheckInteger(-1));
                ++num;
            }
            else if (l.Name == std::string{"y"})
            {
                CHECK_EQ(lua::Integer{6}, L.CheckInteger(-1));
                ++num;
            }
            L.Pop(1);
        }
        CHECK_EQ(3, num);
        L.Debug_PushDebugInfoFunc(i);
        for (const auto& u : L.Debug_Upvalues(-1))
        {
            if (u.Name == std::string_view{"upv"})
            {
                CHECK_EQ(lua::Integer{0}, L.CheckInteger(-1));
                L.Push(4242);
                CHECK_EQ(std::string_view{"upv"}, L.Debug_SetUpvalue(-3, u.UpvalNum));
                ++num;
            }
            else if (u.Name == std::string{"u2"})
            {
                CHECK_EQ(lua::Integer{4}, L.CheckInteger(-1));
                ++num;
            }
            else if (u.Name == std::string{"u3"})
            {
                CHECK_EQ(lua::Integer{5}, L.CheckInteger(-1));
                ++num;
            }
            L.Pop(1);
        }
        CHECK_EQ(6, num);
        return 0;
    }
    TEST_CASE_TEMPLATE("LocalAccess", S, lua::UniqueState, ExtUniqueState)
    {
        S L{};

        CHECK_EQ(0, L.GetTop());
        CHECK_EQ(0, L.Debug_GetStackDepth());

        L.Push("black_magic");
        L.template Push<black_magic<typename S::Base>>();
        L.SetGlobal();

        L.DoStringT("local upv = 0; local u2=4; local u3=5; function foo() local y = 6; local l = 1; local x = 5; "
                    "black_magic(); u3,u2=u2,u3; return l,upv; end; local a,b = foo(); return a,b;");
        CHECK_EQ(2, L.GetTop());
        CHECK_EQ(lua::Integer{42}, L.CheckInteger(1));
        CHECK_EQ(lua::Integer{4242}, L.CheckInteger(2));
    }
}