cmake_minimum_required(VERSION 4.0)
project(lua50_builder)

enable_language(C)

add_library(lua50
        lua/src/lapi.c
        lua/src/lcode.c
        lua/src/ldebug.c
        lua/src/ldo.c
        lua/src/ldump.c
        lua/src/lfunc.c
        lua/src/lgc.c
        lua/src/llex.c
        lua/src/lmem.c
        lua/src/lobject.c
        lua/src/lopcodes.c
        lua/src/lparser.c
        lua/src/lstate.c
        lua/src/lstring.c
        lua/src/ltable.c
        lua/src/ltests.c
        lua/src/ltm.c
        lua/src/lundump.c
        lua/src/lvm.c
        lua/src/lzio.c
        lua/src/lib/lauxlib.c
        lua/src/lib/lbaselib.c
        lua/src/lib/ldblib.c
        lua/src/lib/liolib.c
        lua/src/lib/lmathlib.c
        lua/src/lib/loadlib.c
        lua/src/lib/lstrlib.c
        lua/src/lib/ltablib.c
)

target_include_directories(lua50
        PUBLIC
        lua/src
        lua/include
)
