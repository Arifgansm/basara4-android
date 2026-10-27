/* Basara 4 Android - Probe Loader Entry Point */
/*
 * This is the main entry point that loads the game's eboot.bin,
 * sets up the guest CPU environment, and starts execution.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>
#include "runtime.h"
#include "guest_cpu.h"

#define LOG_TAG "Basara4Probe"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* External functions from runtime modules */
extern int runtime_init(void);
extern void runtime_shutdown(void);
extern uintptr_t runtime_thread_create(void* (*start)(void*), void* arg, size_t stack_size);

/* Game preparation functions (from scripts) */
extern int prepare_game_image(const char* eboot_path, uint8_t** image, size_t* size);
extern int link_game_libc(uint8_t* image, size_t size);

/* Main entry point called by Android launcher */
int main(int argc, char** argv) {
    LOGI("Basara 4 Android - Starting...");
    
    /* Initialize runtime */
    if (runtime_init() != 0) {
        LOGE("Failed to initialize runtime");
        return 1;
    }
    
    /* Get game path from arguments or environment */
    const char* game_path = NULL;
    if (argc > 1) {
        game_path = argv[1];
    } else {
        game_path = getenv("BASARA4_GAME_PATH");
    }
    
    if (!game_path) {
        LOGE("No game path specified");
        runtime_shutdown();
        return 1;
    }
    
    LOGI("Game path: %s", game_path);
    
    /* Prepare game image */
    uint8_t* game_image = NULL;
    size_t game_size = 0;
    
    if (prepare_game_image(game_path, &game_image, &game_size) != 0) {
        LOGE("Failed to prepare game image");
        runtime_shutdown();
        return 1;
    }
    
    LOGI("Game image loaded: %zu bytes", game_size);
    
    /* Link game's bundled libc */
    if (link_game_libc(game_image, game_size) != 0) {
        LOGE("Failed to link game libc");
        free(game_image);
        runtime_shutdown();
        return 1;
    }
    
    /* Setup guest CPU */
    guest_cpu_code((uintptr_t)game_image, game_size);
    
    /* Set up host functions for guest to call */
    /* These would be registered with guest_cpu_host_function() */
    
    /* Start game execution */
    /* The game's entry point would be calculated from the ELF */
    uintptr_t entry_point = 0x10000; /* Placeholder - would parse ELF */
    
    LOGI("Starting game at entry point: 0x%llx", (unsigned long long)entry_point);
    
    /* Execute game entry point */
    guest_cpu_thread_stack(NULL, 0);
    
    /* Call game entry point via FEXCore */
    uint64_t args[] = {0};
    uint64_t result = guest_cpu_call(entry_point, 1, args);
    
    LOGI("Game exited with code: %llu", (unsigned long long)result);
    
    /* Cleanup */
    free(game_image);
    runtime_shutdown();
    
    return 0;
}
