# AdvanceMENU 2.9 MP4 — Windows x86 build

[English](BUILD.md) | [Español](BUILD.es.md)

## 1. Environment

Use the **MSYS2 MINGW32** shell, not MINGW64 or UCRT64. This guide reconstructs the tested environment; a clean installation has not been verified end to end. MINGW32 is a legacy environment and package availability may vary. Do not substitute 64-bit libraries. Install GCC/G++, binutils, make, NASM, diffutils, patch, pkgconf, tar, xz and Git. Static WinPthread and iconv libraries are also needed.

```sh
gcc -dumpmachine # i686-w64-mingw32
gcc --version
nasm -v
make --version
ls /mingw32/lib/libwinpthread.a /mingw32/lib/libiconv.a
```

## 2. Static libraries

Download the official [FFmpeg 7.1.1](https://ffmpeg.org/releases/ffmpeg-7.1.1.tar.xz) and [SDL 1.2.15](https://www.libsdl.org/release/SDL-1.2.15.tar.gz) source archives. Run from a working directory containing both archives. Do not reuse a tree configured for shared libraries.

```sh
tar -xf ffmpeg-7.1.1.tar.xz
cd ffmpeg-7.1.1
./configure --prefix=/mingw32/ffmpeg-static \
  --arch=x86 --target-os=mingw32 \
  --disable-shared --enable-static --disable-programs \
  --disable-doc --disable-network --disable-autodetect \
  --disable-avdevice --disable-avfilter --disable-postproc \
  --disable-encoders --enable-decoder=h264 --enable-decoder=aac \
  --enable-demuxer=mov --enable-swscale --enable-swresample
make -j4 && make install
cd ..
tar -xf SDL-1.2.15.tar.gz
cd SDL-1.2.15
./configure --prefix=/mingw32/sdl-static \
  --host=i686-w64-mingw32 --enable-static --disable-shared
make -j4 && make install
cd ..
```

```sh
ls /mingw32/ffmpeg-static/lib/lib{avformat,avcodec,avutil,swscale,swresample}.a
ls /mingw32/sdl-static/lib/libSDL{,main}.a
```

## 3. Build the frontend

From the repository root, use the included `compilar.sh`. The Makefile should retain `-O3` and `advance/menu.mak` the static archive paths. Do not run frontend `configure` without checking the configuration, as it may replace the modified Makefile.

The 2.9 script uses static SDL headers; if FFmpeg headers are not also installed in the general prefix, append `-I/mingw32/ffmpeg-static/include` to `SDLCFLAGS` inside `compilar.sh`.

```sh
set -o pipefail
bash compilar.sh 2>&1 | tee compilacion.log
ls -lh obj/menu/windows/blend/advmenu.exe
objdump -p obj/menu/windows/blend/advmenu.exe | grep 'DLL Name'
```

## 4. Verification and troubleshooting

Expected output: `obj/menu/windows/blend/advmenu.exe`. Check that there are no direct imports of SDL, FFmpeg, GCC, C++ or WinPthread DLLs. Test horizontal and vertical MP4s, the 240 × 256 case, AAC audio, looping, rapid navigation and MNG/MP3/WAV playback. Retain FIX3 (`av_force_cpu_flags(0)`) for the stable behavior. For build failures, search the log for `error:` and `undefined reference`.

## 5. Distribution

Before publishing binaries, review all static dependency licenses, preserve notices, provide corresponding source and comply with relinking requirements where applicable. See [CREDITS.md](CREDITS.md).
