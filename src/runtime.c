/* Basara 4 Android - Main Runtime */
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#define LOG_TAG "Basara4Runtime"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Basara4Runtime g_runtime = {0};

/* Android-specific memory allocation */
static void* android_low_map(size_t size, int prot) {
    void* ptr = mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return NULL;
    return ptr;
}

int runtime_init(void) {
    LOGI("Initializing Basara 4 Android runtime...");
    
    memset(&g_runtime, 0, sizeof(g_runtime));
    
    /* Initialize memory manager */
    g_runtime.memory.base = android_low_map(64ULL * 1024 * 1024 * 1024, 
                                             PROT_READ | PROT_WRITE);
    if (!g_runtime.memory.base) {
        LOGE("Failed to allocate guest memory space");
        return -1;
    }
    g_runtime.memory.size = 64ULL * 1024 * 1024 * 1024;
    
    /* Initialize default display settings */
    g_runtime.display.width = 1920;
    g_runtime.display.height = 1080;
    g_runtime.display.refresh_rate = 60;
    g_runtime.display.vsync = true;
    
    /* Initialize default audio settings */
    g_runtime.audio.enabled = true;
    g_runtime.audio.volume = 0.8f;
    g_runtime.audio.sample_rate = 48000;
    g_runtime.audio.buffer_size = 2048;
    
    /* Initialize CPU interface */
    guest_cpu_init(android_low_map);
    
    g_runtime.initialized = true;
    LOGI("Runtime initialized successfully");
    
    return 0;
}

void runtime_shutdown(void) {
    if (!g_runtime.initialized) return;
    
    LOGI("Shutting down runtime...");
    
    g_runtime.running = false;
    
    /* Cleanup GPU */
    // gpu_shutdown();
    
    /* Cleanup audio */
    // audio_shutdown();
    
    /* Free memory */
    if (g_runtime.memory.base) {
        munmap(g_runtime.memory.base, g_runtime.memory.size);
        g_runtime.memory.base = NULL;
    }
    
    g_runtime.initialized = false;
    LOGI("Runtime shutdown complete");
}

/* Thread wrapper for guest threads */
typedef struct {
    void* (*start_routine)(void*);
    void* arg;
} ThreadWrapper;

static void* thread_wrapper(void* arg) {
    ThreadWrapper* tw = (ThreadWrapper*)arg;
    void* (*start)(void*) = tw->start_routine;
    void* thread_arg = tw->arg;
    free(tw);
    
    guest_cpu_thread_stack(NULL, 0); /* Will be set properly by caller */
    
    void* result = start(thread_arg);
    guest_cpu_abandon();
    return result;
}

uintptr_t runtime_thread_create(void* (*start_routine)(void*), void* arg, 
                                 size_t stack_size) {
    if (!g_runtime.initialized) return 0;
    
    /* Allocate stack */
    void* stack = android_low_map(stack_size, PROT_READ | PROT_WRITE);
    if (!stack) {
        LOGE("Failed to allocate thread stack");
        return 0;
    }
    
    /* Set guest thread stack */
    guest_cpu_thread_stack(stack, stack_size);
    
    /* Create host thread */
    ThreadWrapper* tw = malloc(sizeof(ThreadWrapper));
    tw->start_routine = start_routine;
    tw->arg = arg;
    
    pthread_t pthread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr, stack, stack_size);
    
    if (pthread_create(&pthread, &attr, thread_wrapper, tw) != 0) {
        LOGE("Failed to create thread");
        free(tw);
        return 0;
    }
    
    pthread_detach(pthread);
    return (uintptr_t)pthread;
}

void runtime_thread_join(uintptr_t thread) {
    /* Simplified - real impl would track pthread_t */
}

void runtime_thread_exit(void) {
    pthread_exit(NULL);
}

/* Memory management */
void* runtime_malloc(size_t size) {
    if (!g_runtime.initialized) return NULL;
    
    void* ptr = malloc(size);
    if (ptr) {
        g_runtime.memory.total_allocated += size;
    }
    return ptr;
}

void runtime_free(void* ptr) {
    if (!ptr) return;
    free(ptr);
}

void* runtime_mmap(void* addr, size_t length, int prot, int flags, 
                   int fd, off_t offset) {
    return mmap(addr, length, prot, flags, fd, offset);
}

int runtime_munmap(void* addr, size_t length) {
    return munmap(addr, length);
}

/* File operations (Android passthrough) */
int runtime_open(const char* pathname, int flags, ...) {
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, int);
    va_end(args);
    return open(pathname, flags, mode);
}

ssize_t runtime_read(int fd, void* buf, size_t count) {
    return read(fd, buf, count);
}

ssize_t runtime_write(int fd, const void* buf, size_t count) {
    return write(fd, buf, count);
}

int runtime_close(int fd) {
    return close(fd);
}

off_t runtime_lseek(int fd, off_t offset, int whence) {
    return lseek(fd, offset, whence);
}

/* Time functions */
int runtime_clock_gettime(int clock_id, struct timespec* tp) {
    return clock_gettime(clock_id, tp);
}

uint64_t runtime_get_ticks(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* Logging */
void runtime_log(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_DEBUG, tag, fmt, args);
    va_end(args);
}

void runtime_log_set_level(int level) {
    /* TODO: implement log level filtering */
}

/* Gamepad input */
void runtime_pad_update(int pad_index, const uint8_t* buttons,
                        const int16_t* lstick, const int16_t* rstick,
                        uint8_t ltrigger, uint8_t rtrigger) {
    if (pad_index < 0 || pad_index >= MAX_GAMEPADS) return;
    
    memcpy(g_runtime.pads.buttons[pad_index], buttons, 32);
    if (lstick) {
        g_runtime.pads.lsticks[pad_index][0] = lstick[0];
        g_runtime.pads.lsticks[pad_index][1] = lstick[1];
    }
    if (rstick) {
        g_runtime.pads.rsticks[pad_index][0] = rstick[0];
        g_runtime.pads.rsticks[pad_index][1] = rstick[1];
    }
    g_runtime.pads.ltiggers[pad_index] = ltrigger;
    g_runtime.pads.rtiggers[pad_index] = rtrigger;
}

/* Audio callback stub */
void runtime_audio_callback(void* userdata, void* buffer, int buffer_size) {
    /* Real implementation in runtime_audio.c */
}
