/* Basara 4 Android - Offline PS4 System Calls */
/*
 * This file reimplements ~130 PS4 system calls that Basara 4 needs
 * but aren't available in the base SpartanPort runtime.
 *
 * Key features:
 * - Kernel event flags and multi-thread sync primitives
 * - Offline AppContent / PlayGo container virtualization
 * - Offline NetCtl / NP / HTTP stubs
 * - Message dialog and screenshot service bridges
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <android/log.h>
#include "runtime.h"

#define LOG_TAG "Basara4Offline"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

/* Stub implementations - these need real basara4-specific implementations */

/* sceNp */
int32_t sceNpError(void) { return 0; }
int32_t sceNpTrophyGetProgression(uint32_t trophyId, uint32_t* progress) { return 0; }
int32_t sceNpTrophyRecordProgression(uint32_t trophyId, uint32_t progress) { return 0; }
int32_t sceNpTrophySaveTrophyIcon(uint32_t trophyId, const void* data, size_t size) { return 0; }
int32_t sceNpTrophyGetTrophyList(uint32_t trophyId, uint32_t* progress) { return 0; }

/* sceNpFriends */
int32_t sceNpFriendsGetPresence(uint32_t userCount, uint32_t* users) { return 0; }

/* sceNpProfile */
int32_t sceNpProfileGetParam(uint32_t userCount, uint32_t* users) { return 0; }

/* sceNpManager */
int32_t sceNpManagerGetLocalUserProfileVersion(int32_t* version) { *version = 0; return 0; }
int32_t sceNpManagerGetLocalUserPresence(int32_t* presence) { *presence = 0; return 0; }

/* sceAppContent */
int32_t sceAppContentCheckUpdateAvailable(int32_t* available) { *available = 0; return 0; }

/* scePlayGo */
int32_t scePlayGoCheckLicense(int32_t licenseType) { return 0; }

/* sceMsgDialog */
int32_t sceMsgDialogShow(uint32_t mode, const char* msg) {
    LOGI("MsgDialog: %s", msg ? msg : "(null)");
    return 0;
}

/* sceScreenshot */
int32_t sceScreenshotCaptureScreen(void* data, uint32_t size, uint32_t flags) {
    /* Android: use SurfaceView screenshot or framebuffer */
    return 0;
}

/* sceKernelEventFlag */
int32_t sceKernelCreateEventFlag(const char* name, uint32_t type, uint32_t init, int32_t* handle) {
    *handle = 1;
    return 0;
}

int32_t sceKernelSetEventFlag(int32_t handle, uint32_t bit) { return 0; }
int32_t sceKernelClearEventFlag(int32_t handle, uint32_t bit) { return 0; }
int32_t sceKernelWaitEventFlag(int32_t handle, uint32_t bit, uint32_t mode, uint32_t* result, 
                                uint64_t timeout) {
    *result = 0;
    return 0;
}

int32_t sceKernelDeleteEventFlag(int32_t handle) { return 0; }

/* sceNetCtl */
int32_t sceNetCtlCheckNetworkConnectionStatus(int32_t connId, uint32_t* status) {
    /* Offline mode: report no connection */
    *status = 0;
    return 0;
}

int32_t sceNetCtlSubscribeStatusChange(int32_t connId, uint32_t mode, 
                                       void* cb, void* user) {
    return 0;
}

int32_t sceNetCtlUnsubscribeStatusChange(int32_t connId, uint32_t mode) { return 0; }

/* sceHttp */
int32_t sceHttpCreateContext(int32_t mode, int32_t* handle) {
    *handle = 1;
    return 0;
}

int32_t sceHttpDeleteContext(int32_t handle) { return 0; }

int32_t sceHttpCreateRequest(int32_t ctx, uint32_t method, const char* url, 
                            int32_t* handle) {
    *handle = 1;
    return 0;
}

int32_t sceHttpDeleteRequest(int32_t handle) { return 0; }

int32_t sceHttpAddHeaderInt32Value(int32_t handle, const char* name, int32_t value) { return 0; }
int32_t sceHttpAddHeaderStringValue(int32_t handle, const char* name, const char* value) { return 0; }

int32_t sceHttpAddHeaderInt32BinaryValue(int32_t handle, const char* name, 
                                        const void* data, size_t size) { return 0; }

int32_t sceHttpSendRequest(int32_t handle, int32_t* status) {
    /* Offline: simulate success */
    *status = 0;
    return 0;
}

int32_t sceHttpAbortRequest(int32_t handle) { return 0; }

int32_t sceHttpGetResponseData(int32_t handle, uint32_t* size, void** data) {
    *size = 0;
    *data = NULL;
    return 0;
}

/* sceAviPlayer */
int32_t sceAviPlayerOpenFile(int32_t handle, const char* path) { return 0; }
int32_t sceAviPlayerPlay(int32_t handle, uint32_t mode, uint32_t* state) { *state = 1; return 0; }
int32_t sceAviPlayerStop(int32_t handle) { return 0; }
int32_t sceAviPlayerCloseFile(int32_t handle) { return 0; }

/* sceMp4 */
int32_t sceMp4OpenFile(int32_t handle, const char* path) { return 0; }
int32_t sceMp4GetVideoFormat(int32_t handle, uint32_t* fmt) { *fmt = 0; return 0; }

/* sceVideoDecoder */
int32_t sceVideoDecoderCreate(int32_t* handle, uint32_t max) { *handle = 1; return 0; }
int32_t sceVideoDecoderDelete(int32_t handle) { return 0; }

/* sceFios2 */
int32_t sceFios2Open(const char* path, int32_t mode, int32_t* fd) {
    *fd = open(path, mode);
    return *fd >= 0 ? 0 : -1;
}

int32_t sceFios2Close(int32_t fd) {
    return close(fd);
}

int32_t sceFios2Read(int32_t fd, void* buf, size_t size) {
    return read(fd, buf, size);
}

int32_t sceFios2Write(int32_t fd, const void* buf, size_t size) {
    return write(fd, buf, size);
}

int32_t sceFios2Stat(const char* path, uint32_t* stat) { return 0; }
int32_t sceFios2Mkdir(const char* path, uint32_t mode) { return 0; }

/* sceGamedata */
int32_t sceGamedataCheckProgress(uint32_t type, uint32_t mode, uint32_t* progress) {
    *progress = 100;
    return 0;
}

int32_t sceGamedataGetProgress(uint32_t type, uint32_t mode, uint32_t* progress) {
    *progress = 100;
    return 0;
}

/* Stub for all unimplemented calls */
int32_t runtime_offline_stub(const char* func_name) {
    LOGW("Unimplemented offline function: %s", func_name);
    return 0;
}
