#!/bin/bash

download_lua() {
  local link="$1"
  local out="./lua/$2"
  local tmp="./lua.tar.gz"
  if [ ! -d "$out" ]; then
    echo "downloading"
    wget -O "$tmp" "$link"
    mkdir "$out"
    tar -xf "$tmp" -C "$out"
    rm "$tmp"
    mv "$out/include/"* "$out"
    rm -d "$out/include"
  else
    echo "$2 already exists"
  fi
}

download_lua "https://sourceforge.net/projects/luabinaries/files/5.5.0/Linux%20Libraries/lua-5.5.0_Linux515_64_lib.tar.gz/download" "lua55"
download_lua "https://sourceforge.net/projects/luabinaries/files/5.4.8/Linux%20Libraries/lua-5.4.8_Linux515_64_lib.tar.gz/download" "lua54"
download_lua "https://sourceforge.net/projects/luabinaries/files/5.3.6/Linux%20Libraries/lua-5.3.6_Linux515_64_lib.tar.gz/download" "lua53"
download_lua "https://sourceforge.net/projects/luabinaries/files/5.2.4/Linux%20Libraries/lua-5.2.4_Linux515_64_lib.tar.gz/download" "lua52"
download_lua "https://sourceforge.net/projects/luabinaries/files/5.1.5/Linux%20Libraries/lua-5.1.5_Linux515_64_lib.tar.gz/download" "lua51"
download_lua "https://sourceforge.net/projects/luabinaries/files/5.0.3/Linux%20Libraries/lua5_0_3_Linux26g4_64_lib.tar.gz/download" "lua50"

if [ ! -d "./LuappDev/luajit" ]; then
  cd "./luajit_src/src" || exit 1
  make all
  cd "../.." || exit 1
  mkdir "./lua/luajit"
  cp "./luajit_src/src/libluajit.a" "./LuappDev/luajit"
  cp "./luajit_src/src/lua.h" "./LuappDev/luajit"
  cp "./luajit_src/src/lauxlib.h" "./LuappDev/luajit"
  cp "./luajit_src/src/lualib.h" "./LuappDev/luajit"
  cp "./luajit_src/src/luajit.h" "./LuappDev/luajit"
  cp "./luajit_src/src/luaconf.h" "./LuappDev/luajit"
else
  echo "luajit already exists"
fi