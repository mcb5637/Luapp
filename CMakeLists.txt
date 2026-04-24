cmake_minimum_required(VERSION 4.0)

project(luapp)

include(CTest)

enable_language(CXX ASM)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(doctest REQUIRED)

set(MSVC_WARNINGS
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
        # be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
        # the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
)
set(CLANG_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
        # catch hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wpedantic # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
)
set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
        -Wsuggest-override # warn if an overridden member function is not marked 'override' or 'final'
        -Wno-changes-meaning
)
list(APPEND CLANG_WARNINGS -Werror)
list(APPEND GCC_WARNINGS -Werror)
list(APPEND MSVC_WARNINGS /WX)
if(MSVC)
    set(PROJECT_WARNINGS_CXX ${MSVC_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS_CXX ${CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS_CXX ${GCC_WARNINGS})
else()
    message(AUTHOR_WARNING "No compiler warnings set for CXX compiler: '${CMAKE_CXX_COMPILER_ID}'")
    # TODO support Intel compiler
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LIB_EXTENSION ".lib")
    set(LIB_PREFIX "")
else()
    set(LIB_EXTENSION ".a")
    set(LIB_PREFIX "lib")
endif()

add_compile_options(${PROJECT_WARNINGS_CXX})

function(make_lua_version lib_name lib_files link include test_name test_opt)
    add_library(${lib_name}
            ${lib_files}
    )

    target_include_directories(${lib_name}
            PUBLIC
            include
            PRIVATE
            ${include}
    )

    target_link_libraries(${lib_name}
            PRIVATE
            ${link}
    )

    if(BUILD_TESTING)
        add_executable(${test_name}
                tests/CreateDelete.cpp
                tests/StackAccess.cpp
                tests/BaseDatatypes.cpp
                tests/Functions.cpp
                tests/Tables.cpp
                tests/Operators.cpp
                tests/RunLua.cpp
                tests/UserClass.cpp
                tests/Iterate.cpp
                tests/LocalAccess.cpp
                tests/Hook.cpp
                tests/Extension.cpp
                tests/AutoCleanStack.cpp
                tests/Typename.cpp
                tests/Serialization.cpp
        )

        target_include_directories(${test_name}
                PUBLIC
                tests
        )

        target_compile_definitions(${test_name} PUBLIC ${test_opt})

        add_test(NAME ${test_name} COMMAND ${test_name})

        target_link_libraries(${test_name} PRIVATE doctest::doctest ${lib_name})
    endif()
endfunction()

make_lua_version("luapp55" "src55/luapp55.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua55/${LIB_PREFIX}lua55${LIB_EXTENSION}" "lua/lua55" "LuappDev55" "-DLUA55")
make_lua_version("luapp54" "src54/luapp54.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua54/${LIB_PREFIX}lua54${LIB_EXTENSION}" "lua/lua54" "LuappDev54" "-DLUA54")
make_lua_version("luapp53" "src53/luapp53.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua53/${LIB_PREFIX}lua53${LIB_EXTENSION}" "lua/lua53" "LuappDev53" "-DLUA53")
make_lua_version("luapp52" "src52/luapp52.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua52/${LIB_PREFIX}lua52${LIB_EXTENSION}" "lua/lua52" "LuappDev52" "-DLUA52")
make_lua_version("luapp51" "src51/luapp51.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua51/${LIB_PREFIX}lua5.1${LIB_EXTENSION}" "lua/lua51" "LuappDev51" "-DLUA51")
make_lua_version("luapp50" "src50/luapp50.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/lua50/${LIB_PREFIX}lua50${LIB_EXTENSION}" "lua/lua50" "LuappDev50" "-DLUA50")
make_lua_version("luappjit" "srcjit/luappjit.cpp;src51/luapp51.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/lua/luajit/${LIB_PREFIX}luajit${LIB_EXTENSION}" "lua/luajit" "LuappDevjit" "-DLUAJIT")
