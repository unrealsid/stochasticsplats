#include <jni.h>
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "NativeDebugBridge", __VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_MainActivity_initNative(
        JNIEnv* env,
        jobject /* this */) {
    LOGD("Native debugger bridge initialized. C++ code is now reachable.");
}
