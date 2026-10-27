/* Basara 4 Android - Runtime Header */
#ifndef BASARA4_RUNTIME_H
#define BASARA4_RUNTIME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Guest CPU interface (defined in guest_cpu.h) */
#include "guest_cpu.h"

/* Maximum number of simultaneous threads */
#define MAX_THREADS 64

/* Maximum gamepad connections */
#define MAX_GAMEPADS 4

/* Runtime state */
typedef struct {
    bool initialized;
    bool running;
    
    /* Thread management */
    struct {
        uintptr_t stacks[MAX_THREADS];
        size_t stack_sizes[MAX_THREADS];
        int active_count;
    } threads;
    
    /* Memory management */
    struct {
        void* base;
        size_t size;
        uint64_t total_allocated;
        uint64_t total_freed;
    } memory;
    
    /* Gamepad state */
    struct {
        uint8_t buttons[MAX_GAMEPADS][32];
        int16_t lsticks[MAX_GAMEPADS][2];
        int16_t rsticks[MAX_GAMEPADS][2];
        uint8_t ltiggers[MAX_GAMEPADS];
        uint8_t rtiggers[MAX_GAMEPADS];
    } pads;
    
    /* Audio state */
    struct {
        bool enabled;
        float volume;
        int sample_rate;
        int buffer_size;
    } audio;
    
    /* Display state */
    struct {
        int width;
        int height;
        int refresh_rate;
        bool vsync;
    } display;
    
    /* File system paths */
    char game_path[512];
    char save_path[512];
    char config_path[512];
    char log_path[512];
} Basara4Runtime;

/* Global runtime instance */
extern Basara4Runtime g_runtime;

/* Initialization */
int runtime_init(void);
void runtime_shutdown(void);

/* Thread management */
uintptr_t runtime_thread_create(void* (*start_routine)(void*), void* arg, size_t stack_size);
void runtime_thread_join(uintptr_t thread);
void runtime_thread_exit(void);

/* Memory management */
void* runtime_malloc(size_t size);
void runtime_free(void* ptr);
void* runtime_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int runtime_munmap(void* addr, size_t length);

/* File system */
int runtime_open(const char* pathname, int flags, ...);
 ssize_t runtime_read(int fd, void* buf, size_t count);
ssize_t runtime_write(int fd, const void* buf, size_t count);
int runtime_close(int fd);
off_t runtime_lseek(int fd, off_t offset, int whence);

/* Time */
int runtime_clock_gettime(int clock_id, struct timespec* tp);
uint64_t runtime_get_ticks(void);

/* Logging */
void runtime_log(const char* tag, const char* fmt, ...);
void runtime_log_set_level(int level);

/* Gamepad input */
void runtime_pad_update(int pad_index, const uint8_t* buttons, 
                        const int16_t* lstick, const int16_t* rstick,
                        uint8_t ltrigger, uint8_t rtrigger);

/* Audio callback (called by Android AudioTrack) */
void runtime_audio_callback(void* userdata, void* buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* BASARA4_RUNTIME_H */
