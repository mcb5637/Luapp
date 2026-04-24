#include <doctest/doctest.h>

#include <luaver.h>

namespace LuappDev
{
#if defined(_MSC_VER) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
    inline std::string_view demangle(std::string_view v)
    {
        return v;
    }
#else
#include <cxxabi.h>
    inline std::string demangle(std::string_view v)
    {
        int status = 0;
        const std::unique_ptr<char, decltype([](char* c) {std::free(c);})> p{
            abi::__cxa_demangle(v.data(), nullptr, nullptr, &status)
        };
        CHECK(status == 0);
        return std::string{p.get()};
    }
#endif

    struct Struct {};
    enum class En : int {};
    class Cls {};
    using Usi = Struct;
    typedef Cls Td;
    using Fun = int(*)(double, float);
    template<class T>
    struct TCls {};
    template<int I>
    struct ICls {};

    TEST_CASE("typename ")
    {
        CHECK(demangle(typeid(Struct).name()) == typename_details::type_name<Struct>());
        CHECK(demangle(typeid(En).name()) == typename_details::type_name<En>());
        CHECK(demangle(typeid(Cls).name()) == typename_details::type_name<Cls>());
        CHECK(demangle(typeid(Usi).name()) == typename_details::type_name<Usi>());
        CHECK(demangle(typeid(Td).name()) == typename_details::type_name<Td>());
#ifdef _MSC_VER
        {
            std::string dem{demangle(typeid(Fun).name())};
            std::erase(dem, ' ');
            std::string ty{typename_details::type_name<Fun>()};
            std::erase(ty, ' ');
            CHECK(dem == ty);
        }
#else
        CHECK(demangle(typeid(Fun).name()) == typename_details::type_name<Fun>());
#endif
        CHECK(demangle(typeid(TCls<int>).name()) == typename_details::type_name<TCls<int>>());
        CHECK(demangle(typeid(ICls<5>).name()) == typename_details::type_name<ICls<5>>());
    }
}