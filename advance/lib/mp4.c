#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/cpu.h>
#include <errno.h>
#include "portable.h"
#include "mp4.h"

struct adv_mp4_struct {
    AVFormatContext* fmt;
    AVCodecContext* codec;
    AVPacket* packet;
    AVFrame* frame;
    struct SwsContext* sws;
    int stream;
    int draining;
    double delay;
    double next_pts;
    double first_pts;
    int have_pts;
};

void adv_mp4_done(adv_mp4* m)
{
    if (!m) return;
    sws_freeContext(m->sws);
    av_frame_free(&m->frame);
    av_packet_free(&m->packet);
    avcodec_free_context(&m->codec);
    avformat_close_input(&m->fmt);
    free(m);
}

adv_mp4* adv_mp4_init(const char* path)
{
    const AVCodec* decoder = NULL;
    AVRational rate;
    adv_mp4* m = calloc(1, sizeof(*m));
    if (!m) return NULL;
    av_force_cpu_flags(0);
    if (avformat_open_input(&m->fmt, path, av_find_input_format("mov"), NULL) < 0) goto fail;
    if (avformat_find_stream_info(m->fmt, NULL) < 0) goto fail;
    m->stream = av_find_best_stream(m->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (m->stream < 0 || !decoder) goto fail;
    m->codec = avcodec_alloc_context3(decoder);
    if (!m->codec) goto fail;
    if (avcodec_parameters_to_context(m->codec, m->fmt->streams[m->stream]->codecpar) < 0) goto fail;
    m->codec->thread_count = 1;
    if (avcodec_open2(m->codec, decoder, NULL) < 0) goto fail;
    m->packet = av_packet_alloc();
    m->frame = av_frame_alloc();
    if (!m->packet || !m->frame) goto fail;
    rate = av_guess_frame_rate(m->fmt, m->fmt->streams[m->stream], NULL);
    m->delay = (rate.num > 0 && rate.den > 0) ? av_q2d(av_inv_q(rate)) : 1.0/30;
    return m;
fail:
    adv_mp4_done(m);
    return NULL;
}

int adv_mp4_read(adv_mp4* m, adv_bitmap** bitmap, double* delay)
{
    int r;
    adv_bitmap* b;
    uint8_t* dst[4] = { NULL, NULL, NULL, NULL };
    int stride[4] = { 0, 0, 0, 0 };
    int w, h;
    *bitmap = NULL;
    for (;;) {
        r = avcodec_receive_frame(m->codec, m->frame);
        if (r == 0) break;
        if (r == AVERROR_EOF) return 0;
        if (r != AVERROR(EAGAIN) || m->draining) return -1;
        do {
            r = av_read_frame(m->fmt, m->packet);
            if (r < 0) break;
            if (m->packet->stream_index == m->stream) break;
            av_packet_unref(m->packet);
        } while (1);
        if (r == AVERROR_EOF) {
            m->draining = 1;
            r = avcodec_send_packet(m->codec, NULL);
        } else if (r >= 0) {
            r = avcodec_send_packet(m->codec, m->packet);
            av_packet_unref(m->packet);
        }
        if (r < 0) return -1;
    }
    w = m->frame->width;
    h = m->frame->height;
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096) return -1;
    m->sws = sws_getCachedContext(m->sws, w, h, m->frame->format,
        w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    if (!m->sws) return -1;
    /* Own a suitably aligned RGB buffer using AdvanceMENU's free convention. */
    if ((w * 3) % 32 == 0) {
        /* Conversion directa optimizada. */
        b = malloc(sizeof(*b));
        if (!b) return -1;

        b->size_x = w;
        b->size_y = h;
        b->bytes_per_pixel = 3;
        b->bytes_per_scanline = w * 3;

        b->heap = malloc(
            (size_t)b->bytes_per_scanline * h + 31 + 64
        );

        if (!b->heap) {
            free(b);
            return -1;
        }

        b->ptr = (uint8_t*)(
            ((uintptr_t)b->heap + 31) & ~(uintptr_t)31
        );

        dst[0] = b->ptr;
        stride[0] = b->bytes_per_scanline;

        if (sws_scale(m->sws,
                (const uint8_t* const*)m->frame->data,
                m->frame->linesize,
                0, h, dst, stride) != h) {
            adv_bitmap_free(b);
            return -1;
        }

    } else {
        /* Modo compatible con resoluciones especiales. */
        AVFrame* rgb;
        int y;

        rgb = av_frame_alloc();
        if (!rgb) return -1;

        rgb->format = AV_PIX_FMT_RGB24;
        rgb->width = w;
        rgb->height = h;

        if (av_frame_get_buffer(rgb, 32) < 0 ||
            sws_scale(m->sws,
                (const uint8_t* const*)m->frame->data,
                m->frame->linesize,
                0, h, rgb->data, rgb->linesize) != h) {
            av_frame_free(&rgb);
            return -1;
        }

        b = adv_bitmap_alloc(w, h, 3);

        if (!b) {
            av_frame_free(&rgb);
            return -1;
        }

        for (y = 0; y < h; ++y) {
            memcpy(
                adv_bitmap_line(b, y),
                rgb->data[0] + y * rgb->linesize[0],
                w * 3
            );
        }

        av_frame_free(&rgb);
    }
    /* Timestamp of this picture, plus its duration, relative to first picture. */
    {
        double duration = m->delay;
        double pts = m->next_pts;
        AVRational tb = m->fmt->streams[m->stream]->time_base;
        if (m->frame->duration > 0) {
            double d = m->frame->duration * av_q2d(tb);
            if (d > 0.001 && d < 1.0) duration = d;
        }
        if (m->frame->best_effort_timestamp != AV_NOPTS_VALUE) {
            double t = m->frame->best_effort_timestamp * av_q2d(tb);
            if (!m->have_pts) { m->first_pts = t - pts; m->have_pts = 1; }
            t -= m->first_pts;
            if (t >= 0 && t > pts - 1.0 && t < pts + 1.0) pts = t;
        }
        m->next_pts = pts + duration;
        *delay = duration;
    }
    av_frame_unref(m->frame);
    *bitmap = b;
    return 1;
}

double adv_mp4_next_pts(const adv_mp4* m) { return m->next_pts; }

/* Decode only as much audio as the mixer requests. */
#define MP4_AUDIO_RATE 44100
#define MP4_AUDIO_BLOCK 16384

struct mp4_audio_stream {
    AVFormatContext* fmt;
    AVCodecContext* codec;
    AVPacket* packet;
    AVFrame* frame;
    SwrContext* swr;
    char* path;
    int stream, draining, eof, loop;
    unsigned pos, count;
    unsigned char pcm[MP4_AUDIO_BLOCK * 4];
};

static void mp4_audio_release(struct mp4_audio_stream* a)
{
    swr_free(&a->swr);
    av_frame_free(&a->frame);
    av_packet_free(&a->packet);
    avcodec_free_context(&a->codec);
    avformat_close_input(&a->fmt);
}

void adv_mp4_audio_close(void* opaque)
{
    struct mp4_audio_stream* a = opaque;
    if (!a) return;
    mp4_audio_release(a);
    free(a->path);
    free(a);
}

static int mp4_audio_start(struct mp4_audio_stream* a)
{
    const AVCodec* decoder = NULL;
    a->draining = a->eof = 0;
    a->pos = a->count = 0;
    if (avformat_open_input(&a->fmt, a->path, av_find_input_format("mov"), NULL) < 0) return -1;
    if (avformat_find_stream_info(a->fmt, NULL) < 0) return -1;
    a->stream = av_find_best_stream(a->fmt, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    if (a->stream < 0 || !decoder) return -1;
    a->codec = avcodec_alloc_context3(decoder);
    if (!a->codec || avcodec_parameters_to_context(a->codec, a->fmt->streams[a->stream]->codecpar) < 0) return -1;
    a->codec->thread_count = 1;
    if (avcodec_open2(a->codec, decoder, NULL) < 0) return -1;
    a->packet = av_packet_alloc();
    a->frame = av_frame_alloc();
    return a->packet && a->frame ? 0 : -1;
}

void* adv_mp4_audio_open(const char* path, int loop)
{
    struct mp4_audio_stream* a = calloc(1, sizeof(*a));
    if (!a) return NULL;
    a->path = malloc(strlen(path) + 1);
    if (!a->path) { adv_mp4_audio_close(a); return NULL; }
    strcpy(a->path, path);
    a->loop = loop;
    if (mp4_audio_start(a) < 0) { adv_mp4_audio_close(a); return NULL; }
    return a;
}

/* Return samples in one decoded block, 0 at EOF, -1 on failure. */
static int mp4_audio_block(struct mp4_audio_stream* a)
{
    uint8_t* out = a->pcm;
    int r;
    for (;;) {
        if (a->eof) {
            if (!a->swr) return 0;
            return swr_convert(a->swr, &out, MP4_AUDIO_BLOCK, NULL, 0);
        }
        r = avcodec_receive_frame(a->codec, a->frame);
        if (r == 0) {
            if (!a->swr) {
                AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
                if (a->frame->sample_rate <= 0 || !av_channel_layout_check(&a->frame->ch_layout)) return -1;
                if (swr_alloc_set_opts2(&a->swr, &stereo, AV_SAMPLE_FMT_S16,
                    MP4_AUDIO_RATE, &a->frame->ch_layout, a->frame->format,
                    a->frame->sample_rate, 0, NULL) < 0 || swr_init(a->swr) < 0) return -1;
            }
            r = swr_get_out_samples(a->swr, a->frame->nb_samples);
            if (r < 0 || r > MP4_AUDIO_BLOCK) return -1;
            r = swr_convert(a->swr, &out, MP4_AUDIO_BLOCK,
                (const uint8_t**)a->frame->extended_data, a->frame->nb_samples);
            av_frame_unref(a->frame);
            if (r != 0) return r;
            continue;
        }
        if (r == AVERROR_EOF) { a->eof = 1; continue; }
        if (r != AVERROR(EAGAIN) || a->draining) return -1;
        while ((r = av_read_frame(a->fmt, a->packet)) >= 0) {
            if (a->packet->stream_index == a->stream) break;
            av_packet_unref(a->packet);
        }
        if (r == AVERROR_EOF) {
            a->draining = 1;
            r = avcodec_send_packet(a->codec, NULL);
        } else if (r >= 0) {
            r = avcodec_send_packet(a->codec, a->packet);
            av_packet_unref(a->packet);
        }
        if (r < 0) return -1;
    }
}

/* Return up to max stereo sample frames. Loop once per empty read at most. */
int adv_mp4_audio_read(void* opaque, unsigned char* data, unsigned max)
{
    struct mp4_audio_stream* a = opaque;
    int r;
    unsigned n;
    if (!max) return 0;
    if (a->pos == a->count) {
        r = mp4_audio_block(a);
        if (r == 0 && a->loop) {
            mp4_audio_release(a);
            if (mp4_audio_start(a) < 0) return -1;
            r = mp4_audio_block(a);
        }
        if (r <= 0) return r;
        a->pos = 0;
        a->count = r;
    }
    n = a->count - a->pos;
    if (n > max) n = max;
    memcpy(data, a->pcm + a->pos * 4, n * 4);
    a->pos += n;
    return n;
}
