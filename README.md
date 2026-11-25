# AdvanceMENU 2.9 MP4

[English](README.md) | [Español](README.es.md)

An unofficial arcade emulator frontend derived from AdvanceMENU 2.9, with native MP4 preview playback using FFmpeg.

## Features

- H.264 video and AAC audio inside MP4 files, without an external player.
- Single-thread video and audio decoding; progressive PCM audio buffering.
- Direct RGB conversion for compatible widths, buffered conversion for other widths.
- Timestamp-based video scheduling and estimated audio-clock synchronization.
- Existing MNG previews and separate MP3/WAV audio remain supported; separate audio has priority.
- GCC `-O3` optimization; static FFmpeg, SDL and WinPthread linking in the Windows x86 build.
- FIX3 compatibility workaround for a crash observed with 240 × 256 previews.

## Usage

Build using [BUILD.md](BUILD.md), or use a binary you have built. Keep a backup of your existing frontend and configuration. Place MP4 previews in the configured snapshot/clip directory, named after the game (for example `blkpnthr.mp4`). Configure that directory in the frontend as usual. For looping single-game previews, try `ui_clip singleloop` in `advmenu.rc`. Existing configuration options are documented in the original `doc/` directory.

## Tested environment and limitations

Windows x86, MSYS2 MINGW32, GCC 16.2.0, FFmpeg 7.1.1 and SDL 1.2.15. The user-built executable was approximately 21 MB; its direct imports were Windows system DLLs only. Playback was reported working locally. No comprehensive regression suite or portable performance benchmark has been run. Other codecs, platforms and CPU configurations are not guaranteed. FIX3 disables runtime-selected FFmpeg SIMD paths and may increase CPU usage. A/V synchronization is approximate, not guaranteed sample-accurate. This is a modified AdvanceMENU 2.9 build, not an official upstream release.

## Documentation and license

- [Build guide](BUILD.md) / [Español](BUILD.es.md)
- [Changelog](CHANGELOG.md)
- [Credits and license notes](CREDITS.md)
- Original documentation: `README`, `BUILD`, `HISTORY` (where present), `doc/`, `COPYING`.

Upstream: [Advance project](https://www.advancemame.it/). Preserve existing copyright notices and license files. See `COPYING` and individual source headers.
