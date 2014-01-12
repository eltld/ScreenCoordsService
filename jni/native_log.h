#ifndef _NATIVE_LOG_H
#define _NATIVE_LOG_H

/******************************************************************
 *
 * IMPORTANT NOTICE:
 *
 *   - usage: LOGX(fmt, ...), where X is log level V, D, I, W, E;
 *   - LOCAL_LDLIBS := -llog MUST add to Android.mk, otherwise report
 *     linked error, more info at <ndk>/docs/ANDROID-MK.html;
 *   - if TAG not defined, "::JNI_LOG" by default;
 *
 */

#include <android/log.h>

#ifndef TAG
#define TAG "JNI_LOG"
#endif

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

void setNativeDebugModeOn();
void setNativeDebugModeOff();
void nativeDebug(const char *fmt, ...);

#endif // _NATIVE_LOG_H
