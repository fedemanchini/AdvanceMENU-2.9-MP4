# AdvanceMENU 2.9 MP4

[English](README.md) | [Español](README.es.md)

Frontend arcade no oficial derivado de AdvanceMENU 2.9, con reproducción nativa de vistas previas MP4 mediante FFmpeg.

## Características

- Video H.264 y audio AAC dentro de MP4, sin reproductor externo.
- Decodificación de video y audio con un hilo; audio PCM progresivo.
- Conversión RGB directa para anchos compatibles y con búfer para otros anchos.
- Temporización por marcas de tiempo y sincronización mediante reloj de audio estimado.
- Conserva vistas previas MNG y audio MP3/WAV independiente, que tiene prioridad.
- Optimización GCC `-O3` y vinculación estática de FFmpeg, SDL y WinPthread en Windows x86.
- Corrección FIX3 para un cierre observado con videos de 240 × 256.

## Uso

Compilá siguiendo [BUILD.es.md](BUILD.es.md). Guardá una copia del frontend y la configuración existentes. Colocá los MP4 en el directorio configurado de capturas o clips, con el nombre del juego (por ejemplo `blkpnthr.mp4`). Configurá ese directorio como de costumbre. Para repetir la vista previa de un juego, probá `ui_clip singleloop` en `advmenu.rc`. Consultá `doc/` para las opciones originales.

## Entorno probado y limitaciones

Windows x86, MSYS2 MINGW32, GCC 16.2.0, FFmpeg 7.1.1 y SDL 1.2.15. El ejecutable compilado localmente ocupó aproximadamente 21 MB y sus importaciones directas fueron únicamente DLL de Windows. Se informó reproducción correcta en el equipo de prueba. No se realizó una batería exhaustiva de regresiones ni mediciones comparables de rendimiento. No se garantiza compatibilidad con otros códecs, plataformas o CPU. FIX3 desactiva las rutas SIMD seleccionadas dinámicamente por FFmpeg y puede aumentar el consumo de CPU. La sincronización audiovisual es aproximada. Esta es una compilación modificada de AdvanceMENU 2.9, no una publicación oficial del proyecto original.

## Documentación y licencia

- [Guía de compilación](BUILD.es.md) / [English](BUILD.md)
- [Historial de cambios](CHANGELOG.md)
- [Créditos y licencias](CREDITS.md)
- Documentación original: `README`, `BUILD`, `HISTORY` (si existe), `doc/`, `COPYING`.

Proyecto original: [Advance](https://www.advancemame.it/). Conservá los avisos de copyright y las licencias originales. Consultá `COPYING` y los encabezados de cada archivo.
