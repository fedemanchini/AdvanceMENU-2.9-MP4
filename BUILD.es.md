# AdvanceMENU 2.9 MP4 — Compilación en Windows x86

[English](BUILD.md) | [Español](BUILD.es.md)

## 1. Entorno

Usar la terminal **MSYS2 MINGW32**, no MINGW64 ni UCRT64. La guía reconstruye el entorno utilizado en las pruebas; una instalación nueva no fue verificada de principio a fin. MINGW32 es un entorno antiguo y la disponibilidad de paquetes puede variar. Si falta un paquete, no sustituirlo por una biblioteca de 64 bits. Instalar GCC/G++, binutils, make, NASM, diffutils, patch, pkgconf, tar, xz y Git. También se requieren las bibliotecas estáticas de WinPthread e iconv.

```sh
gcc -dumpmachine # i686-w64-mingw32
gcc --version
nasm -v
make --version
ls /mingw32/lib/libwinpthread.a /mingw32/lib/libiconv.a
```

## 2. Bibliotecas estáticas

Descargar los archivos fuente oficiales de [FFmpeg 7.1.1](https://ffmpeg.org/releases/ffmpeg-7.1.1.tar.xz) y [SDL 1.2.15](https://www.libsdl.org/release/SDL-1.2.15.tar.gz). Ejecutar desde una carpeta de trabajo que contenga ambos archivos. No reutilizar un árbol configurado para bibliotecas dinámicas.

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

## 3. Compilar el frontend

Desde la raíz del repositorio, utilizar el `compilar.sh` incluido. El Makefile debe conservar `-O3`, y `advance/menu.mak` las rutas a los archivos estáticos. No ejecutar `configure` sobre el frontend sin revisar la configuración, ya que puede reemplazar el Makefile modificado.

El script de 2.9 utiliza las cabeceras SDL estáticas; si FFmpeg no está instalado también en el prefijo general, añadir `-I/mingw32/ffmpeg-static/include` a `SDLCFLAGS` dentro de `compilar.sh`.

```sh
set -o pipefail
bash compilar.sh 2>&1 | tee compilacion.log
ls -lh obj/menu/windows/blend/advmenu.exe
objdump -p obj/menu/windows/blend/advmenu.exe | grep 'DLL Name'
```

## 4. Verificación y diagnóstico

El resultado esperado es `obj/menu/windows/blend/advmenu.exe`. Revisar que no importe directamente DLL de SDL, FFmpeg, GCC, C++ o WinPthread. Probar MP4 horizontal y vertical, el caso 240 × 256, audio AAC, bucles, navegación rápida y reproducción MNG/MP3/WAV. Mantener FIX3 (`av_force_cpu_flags(0)`) para reproducir el comportamiento estable. Si falla la compilación, buscar `error:` y `undefined reference` en el registro.

## 5. Distribución

Antes de publicar binarios, revisar las licencias de todos los componentes estáticos, conservar avisos, proporcionar el código fuente correspondiente y cumplir los requisitos de reenlazado cuando correspondan. Véase [CREDITS.md](CREDITS.md).
