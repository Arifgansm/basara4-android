#include <jni.h>
#include <android/log.h>
#include <string>
#include "runtime.h"
#include "guest_cpu.h"

#define LOG_TAG "Basara4JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

JNIEXPORT void JNICALL
Java_com_basara4_android_nativeinit_NativeLib_initLogging(JNIEnv* env, jclass clazz) {
    LOGI("Native logging initialized");
}

JNIEXPORT void JNICALL
Java_com_basara4_android_nativeinit_NativeLib_onAppForeground(JNIEnv* env, jclass clazz) {
    LOGI("App foreground");
    // Resume audio, graphics
}

JNIEXPORT void JNICALL
Java_com_basara4_android_nativeinit_NativeLib_onAppBackground(JNIEnv* env, jclass clazz) {
    LOGI("App background");
    // Pause audio, graphics
}

JNIEXPORT jboolean JNICALL
Java_com_basara4_android_nativeinit_NativeLib_prepareGameImage(JNIEnv* env, jclass clazz, jstring gamePath) {
    const char* path = env->GetStringUTFChars(gamePath, nullptr);
    if (!path) return JNI_FALSE;
    
    LOGI("Preparing game: %s", path);
    
    // Initialize runtime
    if (runtime_init() != 0) {
        LOGE("Runtime init failed");
        env->ReleaseStringUTFChars(gamePath, path);
        return JNI_FALSE;
    }
    
    // Prepare game image (stub - real impl in scripts)
    // prepare_game_image(path, &image, &size);
    // link_game_libc(image, size);
    
    env->ReleaseStringUTFChars(gamePath, path);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_basara4_android_nativeinit_NativeLib_launchGame(JNIEnv* env, jclass clazz, jstring gamePath) {
    const char* path = env->GetStringUTFChars(gamePath, nullptr);
    if (!path) return JNI_FALSE;
    
    LOGI("Launching game: %s", path);
    
    // Start game in separate thread
    // runtime_thread_create(game_main_thread, (void*)path, 8 * 1024 * 1024);
    
    env->ReleaseStringUTFChars(gamePath, path);
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_basara4_android_nativeinit_NativeLib_setGamepadState(
    JNIEnv* env, jclass clazz, jint padIndex,
    jbyteArray buttons, jshort lstickX, jshort lstickY,
    jshort rstickX, jshort rstickY, jshort ltrigger, jshort rtrigger) {
    
    jbyte* btns = env->GetByteArrayElements(buttons, nullptr);
    uint8_t buttons_arr[32];
    for (int i = 0; i < 32; i++) buttons_arr[i] = btns[i];
    
    int16_t lstick[2] = {lstickX, lstickY};
    int16_t rstick[2] = {rstickX, rstickY};
    
    runtime_pad_update(padIndex, buttons_arr, lstick, rstick, ltrigger, rtrigger);
    
    env->ReleaseByteArrayElements(buttons, btns, JNI_ABORT);
}

JNIEXPORT jintArray JNICALL
Java_com_basara4_android_nativeinit_NativeLib_getGamepadState(JNIEnv* env, jclass clazz, jint padIndex) {
    // Return current gamepad state
    jintArray result = env->NewIntArray(40); // 32 buttons + 2 sticks * 2 + 2 triggers
    return result;
}

}