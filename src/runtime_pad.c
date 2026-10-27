/* Basara 4 Android - Gamepad Input */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include "runtime.h"

#define LOG_TAG "Basara4Pad"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/* PS4-style gamepad state */
typedef struct {
    uint8_t connected;
    uint8_t buttons[32];
    int16_t lstick[2];
    int16_t rstick[2];
    uint8_t ltrigger;
    uint8_t rtrigger;
    uint8_t touch_x;
    uint8_t touch_y;
    uint8_t battery;
} GamepadState;

static GamepadState gamepads[4] = {0};

/* Update from Android input (called by launcher) */
void runtime_pad_update(int pad_index, const uint8_t* buttons, 
                        const int16_t* lstick, const int16_t* rstick,
                        uint8_t ltrigger, uint8_t rtrigger) {
    if (pad_index < 0 || pad_index >= 4) return;
    
    if (buttons) memcpy(gamepads[pad_index].buttons, buttons, 32);
    if (lstick) {
        gamepads[pad_index].lstick[0] = lstick[0];
        gamepads[pad_index].lstick[1] = lstick[1];
    }
    if (rstick) {
        gamepads[pad_index].rstick[0] = rstick[0];
        gamepads[pad_index].rstick[1] = rstick[1];
    }
    gamepads[pad_index].ltrigger = ltrigger;
    gamepads[pad_index].rtrigger = rtrigger;
    gamepads[pad_index].connected = 1;
}

/* Get gamepad state (called by game) */
int32_t scePadRead(int32_t port, uint32_t* buttons, int16_t* lstick, 
                   int16_t* rstick, uint8_t* ltrigger, uint8_t* rtrigger) {
    if (port < 0 || port >= 4 || !gamepads[port].connected) return -1;
    
    if (buttons) *buttons = gamepads[port].buttons[0] | (gamepads[port].buttons[1] << 8);
    if (lstick) {
        lstick[0] = gamepads[port].lstick[0];
        lstick[1] = gamepads[port].lstick[1];
    }
    if (rstick) {
        rstick[0] = gamepads[port].rstick[0];
        rstick[1] = gamepads[port].rstick[1];
    }
    if (ltrigger) *ltrigger = gamepads[port].ltrigger;
    if (rtrigger) *rtrigger = gamepads[port].rtrigger;
    
    return 0;
}

/* Vibration */
int32_t scePadSetVibration(int32_t port, uint8_t small, uint8_t large) {
    if (port < 0 || port >= 4) return -1;
    /* TODO: trigger Android vibration */
    return 0;
}

/* Get gamepad info */
int32_t scePadGetInfo(int32_t port, uint32_t* info) {
    if (port < 0 || port >= 4 || !gamepads[port].connected) return -1;
    *info = 0x01; /* Standard PS4 controller */
    return 0;
}

/* Initialize pad subsystem */
int32_t scePadInit(void) {
    memset(gamepads, 0, sizeof(gamepads));
    LOGI("Pad subsystem initialized");
    return 0;
}

/* Terminate pad subsystem */
void scePadTerm(void) {
    memset(gamepads, 0, sizeof(gamepads));
    LOGI("Pad subsystem terminated");
}