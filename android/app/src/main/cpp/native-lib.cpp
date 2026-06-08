#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include "core/log.h"
#include <semaphore.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "JNI_DEBUG", __VA_ARGS__)

void android_init(JNIEnv* env, jlong gl_context, jobject activity, AAssetManager* asset_manager, const std::string& externalPath);
void android_render();
void android_onSurfaceChanged(int width, int height, int displayRotation);
void android_onResume(JNIEnv* env);
void android_onPause();
void setCameraAccess(bool cameraAccess);

AAssetManager* g_AssetManager = nullptr;
std::string g_ExternalDataPath;
JavaVM* g_JavaVM = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_onSurfaceCreated(JNIEnv *env, jclass clazz, jlong gl_context, jobject activity)
{
    android_init(env, gl_context, activity, g_AssetManager, g_ExternalDataPath );
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_setAssetManager(
        JNIEnv *env, jclass clazz, jobject asset_manager)
{
    g_AssetManager = AAssetManager_fromJava(env, asset_manager);
    LOGD("Native AssetManager set.");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_setExternalDataPath(
        JNIEnv *env, jclass clazz, jstring path)
{
    const char* native_path = env->GetStringUTFChars(path, nullptr);
    g_ExternalDataPath = std::string(native_path);
    env->ReleaseStringUTFChars(path, native_path);
    LOGD("Native ExternalDataPath set to: %s", g_ExternalDataPath.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_onDrawFrame(JNIEnv *env, jclass clazz, jlong gl_context)
{
    android_render();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_onSurfaceChanged(JNIEnv *env, jclass clazz, jint width, jint height, jint display_rotation)
{
    android_onSurfaceChanged(width, height, display_rotation);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_onResume(JNIEnv *env, jclass clazz)
{
    android_onResume(env);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_onPause(JNIEnv *env, jclass clazz)
{
    android_onPause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_the_1render_1box_android_1splatapult_JniInterface_setCameraAccess(JNIEnv *env, jclass clazz, jboolean camera_access)
{
    bool hasAccess = (camera_access == JNI_TRUE);
    setCameraAccess(hasAccess);
}