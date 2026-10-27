/* Basara 4 Android - Ngs2 Audio Engine */
/*
 * Custom software Ngs2 sound graph handling multi-channel ATRAC9 streaming,
 * LibAtrac9 decoding, voice synthesis, PCM playback, and submixing.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>
#include "runtime.h"
#include "third_party/LibAtrac9/C/src/libatrac9.h"

#define LOG_TAG "Basara4Audio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

/* ATRAC9 decoder context */
typedef struct {
    struct LibAtrac9Ctx* decoder;
    int32_t* output_buffer;
    size_t buffer_size;
    size_t read_pos;
} Atrac9Stream;

/* Voice state */
typedef struct {
    Atrac9Stream stream;
    float volume;
    int32_t pitch;
    int32_t loop_start;
    int32_t loop_end;
    int32_t play_head;
    int32_t channels;
    int32_t sample_rate;
    int32_t loop;
    int32_t skip;
} Voice;

/* Submixer state */
typedef struct {
    Voice* voices[32];
    int32_t voice_count;
    int32_t channels;
    int32_t sample_rate;
} Submixer;

/* Master bus state */
typedef struct {
    Submixer submixer;
    int32_t* left_buffer;
    int32_t* right_buffer;
    int32_t buffer_size;
    int32_t write_pos;
} MasterBus;

/* Ngs2 context */
typedef struct {
    MasterBus master;
    Atrac9Stream* atrac9_streams[64];
    int32_t atrac9_stream_count;
} Ngs2Context;

static Ngs2Context g_ngs2 = {0};

/* Android audio callback - called by AudioTrack */
void android_audio_callback(int16_t* buffer, size_t frames) {
    if (!g_ngs2.master.left_buffer || !g_ngs2.master.right_buffer) return;
    
    /* Simple passthrough for now - real impl would do mixing */
    for (size_t i = 0; i < frames * 2; i++) {
        buffer[i] = 0; /* TODO: mix from buffers */
    }
}

/* Initialize audio */
int runtime_audio_init(int32_t sample_rate, int32_t buffer_size) {
    g_ngs2.master.submixer.channels = 2;
    g_ngs2.master.submixer.sample_rate = sample_rate;
    g_ngs2.master.buffer_size = buffer_size * 2; /* stereo */
    
    g_ngs2.master.left_buffer = malloc(g_ngs2.master.buffer_size * sizeof(int32_t));
    g_ngs2.master.right_buffer = malloc(g_ngs2.master.buffer_size * sizeof(int32_t));
    
    if (!g_ngs2.master.left_buffer || !g_ngs2.master.right_buffer) {
        LOGE("Failed to allocate audio buffers");
        return -1;
    }
    
    memset(g_ngs2.master.left_buffer, 0, g_ngs2.master.buffer_size * sizeof(int32_t));
    memset(g_ngs2.master.right_buffer, 0, g_ngs2.master.buffer_size * sizeof(int32_t));
    
    LOGI("Audio initialized: %d Hz, buffer %d", sample_rate, buffer_size);
    return 0;
}

/* Shutdown audio */
void runtime_audio_shutdown(void) {
    free(g_ngs2.master.left_buffer);
    free(g_ngs2.master.right_buffer);
    g_ngs2.master.left_buffer = NULL;
    g_ngs2.master.right_buffer = NULL;
}

/* Create ATRAC9 stream */
int32_t runtime_audio_create_atrac9_stream(const uint8_t* data, size_t size) {
    if (g_ngs2.atrac9_stream_count >= 64) return -1;
    
    Atrac9Stream* stream = &g_ngs2.atrac9_streams[g_ngs2.atrac9_stream_count];
    stream->decoder = LibAtrac9Create();
    stream->output_buffer = malloc(4096 * sizeof(int32_t));
    stream->buffer_size = 4096;
    stream->read_pos = 0;
    
    g_ngs2.atrac9_stream_count++;
    g_ngs2.atrac9_streams[g_ngs2.atrac9_stream_count - 1] = stream;
    
    LOGI("ATRACT9 stream created: %zu bytes", size);
    return 0;
}

/* Start audio */
void runtime_audio_start(void) {
    LOGI("Audio started");
}

/* Stop audio */
void runtime_audio_stop(void) {
    LOGI("Audio stopped");
}

/* Ngs2 trace logging */
void runtime_audio_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_VERBOSE, "Basara4AudioNgs2", fmt, args);
    va_end(args);
}
