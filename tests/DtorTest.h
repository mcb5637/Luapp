#pragma once
#include <functional>

namespace LuappDev::cls
{
    class DtorTest
    {
        std::function<void()> f;

    public:
        explicit DtorTest(std::function<void()> v) : f(std::move(v)) {}

        ~DtorTest() { f(); }
    };
}