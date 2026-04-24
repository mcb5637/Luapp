#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
    template<class S>
    static void hook_magic(S L, lua::ActivationRecord ar)
    {
        if (ar.Matches(lua::HookEvent::Return))
        {
            auto inf = L.Debug_GetInfoFromAR(ar, lua::DebugInfoOptions::Name);
            if (inf.Name && inf.Name == std::string_view{"bar"})
            {
                L.Debug_GetStack(1, inf, lua::DebugInfoOptions::Name, false);
                CHECK_EQ(std::string_view{"foo"}, inf.Name);
                for (const auto& l : L.Debug_Locals(inf))
                {
                    if (l.Name == std::string_view{"l"})
                    {
                        CHECK_EQ(lua::Integer{1}, L.CheckInteger(-1));
                        L.Push(42);
                        CHECK_EQ(std::string_view{"l"}, L.Debug_SetLocal(1, l.LocalNum));
                    }
                    L.Pop(1);
                }
                L.Debug_PushDebugInfoFunc(inf);
                for (const auto& u : L.Debug_Upvalues(-1))
                {
                    if (u.Name == std::string_view{"upv"})
                    {
                        CHECK_EQ(lua::Integer{0}, L.CheckInteger(-1));
                        L.Push(4242);
                        CHECK_EQ(std::string_view{"upv"}, L.Debug_SetUpvalue(-3, u.UpvalNum));
                    }
                    L.Pop(1);
                }
                L.Pop(1);
            }
        }
    }

    TEST_CASE_TEMPLATE("Hook", S, lua::UniqueState, ExtUniqueState)
    {
        S L{};
        CHECK_EQ(0, L.GetTop());

        if constexpr (S::Capabilities::JIT)
            L.SetJITMode(S::JITMode::On);

        L.template Debug_SetHook<hook_magic<typename S::Base>>(lua::HookEvent::Call | lua::HookEvent::Return, 0);

        L.DoStringT("local upv = 0; function bar() end; function foo() local l = 1; bar(); return l,upv; end; local "
                    "a,b = foo(); return a,b;");
        CHECK_EQ(2, L.GetTop());
        CHECK_EQ(lua::Integer{42}, L.CheckInteger(1));
        CHECK_EQ(lua::Integer{4242}, L.CheckInteger(2));
    }

    template<class S>
    static void hook_checkline(S L, lua::ActivationRecord ar)
    {
        int t = L.GetTop();
        if (ar.Matches(lua::HookEvent::Line) && L.Debug_GetNameForStackFunc(0) == "test")
        {
            L.PushLightUserdata(reinterpret_cast<void*>(&hook_checkline<S>));
            L.GetTableRaw(S::REGISTRYINDEX);
            auto* exp = static_cast<int*>(L.ToUserdata(-1));
            CHECK_EQ(*exp, ar.Line());
            ++*exp;
        }
        L.SetTop(t);
    }
    template<class S>
    static int activate_hookline(S L)
    {
        L.template Debug_SetHook<hook_checkline<S>>(lua::HookEvent::Line | lua::HookEvent::Call | lua::HookEvent::Return, 0);
        return 0;
    }
    TEST_CASE_TEMPLATE("HookLine", US, lua::UniqueState, ExtUniqueState)
    {
        using S = US::Base;

        US L{};
        CHECK_EQ(0, L.GetTop());

        int exp = lua::State::Version() == 5.0 ? 8 : 7;

        L.PushLightUserdata(reinterpret_cast<void*>(&hook_checkline<S>));
        L.PushLightUserdata(&exp);
        L.SetTableRaw(lua::State::REGISTRYINDEX);
        L.template RegisterFunc<activate_hookline<S>>("activate_hook");
        L.template Debug_SetHook<hook_checkline<S>>(lua::HookEvent::Count, 5000);
        L.DoStringT("function foo() end\n"
                    "\n"
                    "function test()\n"
                    "foo()\n"
                    "activate_hook()\n"
                    "\n"
                    "foo()\n" // 7
                    "foo()\n"
                    "foo()\n"
                    "foo()\n"
                    "foo()\n"
                    "end\n"
                    "test()\n"); // 13
        CHECK_EQ(13, exp);
    }
}