#!/bin/bash

set -e
cd "$(dirname "$0")"

make -j4 \
CONF_BUILD=windows \
CONF_HOST=windows \
CONF_LIB_SDL=yes \
CONF_LIB_FREETYPE=no \
CONF_LIB_ZLIB=no \
CONF_LIB_EXPAT=no \
ASM=nasm \
LD_FOR_BUILD=gcc \
LDXX_FOR_BUILD=g++ \
SDLCFLAGS="-I/mingw32/sdl-static/include/SDL" \
SDLLIBS="-lmingw32 /mingw32/sdl-static/lib/libSDLmain.a /mingw32/sdl-static/lib/libSDL.a -mwindows -Wl,-Bstatic -liconv -Wl,-Bdynamic -lm -luser32 -lgdi32 -lwinmm -ldxguid" \
CONF_LDFLAGS="-s -static-libgcc -static-libstdc++" \
obj/menu/windows/blend/advmenu.exe

echo "Compilacion finalizada correctamente"

ls -lh obj/menu/windows/blend/advmenu.exe

objdump -p obj/menu/windows/blend/advmenu.exe | grep "DLL Name"