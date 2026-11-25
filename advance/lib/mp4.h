#ifndef ADV_MP4_H
#define ADV_MP4_H
#include "bitmap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct adv_mp4_struct adv_mp4;
adv_mp4* adv_mp4_init(const char* path);
void adv_mp4_done(adv_mp4* m);
double adv_mp4_next_pts(const adv_mp4* m);
/* Streaming stereo S16 PCM at 44100 Hz. */
void* adv_mp4_audio_open(const char* path, int loop);
int adv_mp4_audio_read(void* opaque, unsigned char* data, unsigned max);
void adv_mp4_audio_close(void* opaque);
/* Returns 1 for a frame, 0 for EOF, -1 for error. */
int adv_mp4_read(adv_mp4* m, adv_bitmap** bitmap, double* delay);
#ifdef __cplusplus
}
#endif
#endif
