# Changelog / Historial de cambios

## MP4 STABLE — November 2026 / Noviembre de 2026

| Change | Cambio |
|---|---|
| Native MP4/H.264 and AAC playback | Reproducción nativa MP4/H.264 y AAC |
| One decoder thread per context | Un hilo por contexto de decodificación |
| Direct aligned RGB output with compatible fallback | Salida RGB alineada con alternativa compatible |
| Progressive audio decoding instead of full-track buffering | Audio progresivo en lugar de cargar toda la pista |
| PTS-based scheduling and estimated audio clock | Temporización PTS y reloj de audio estimado |
| FIX3: runtime SIMD selection disabled after H.264 crash | FIX3: selección SIMD desactivada tras cierre H.264 |
| Static Windows x86 build and GCC -O3 | Compilación Windows x86 estática y GCC -O3 |

Video initialization applies FIX3. / La inicialización de video aplica FIX3.

This is a modified AdvanceMENU 2.9 build, not an official upstream release.

Esta es una compilación modificada de AdvanceMENU 2.9, no una publicación oficial del proyecto original.

Local playback was reported successful; no complete benchmark or regression suite. / Se informó reproducción correcta localmente; no hay benchmark ni pruebas de regresión exhaustivas.
