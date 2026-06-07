#include "ar_core_manager.h"
#include <android/log.h>
#include <glm/gtc/type_ptr.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define LOG_TAG "ARCoreManager"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

ARCoreManager::ARCoreManager() {}

ARCoreManager::~ARCoreManager() {
    Destroy();
}

bool ARCoreManager::Initialize(JNIEnv* env, void* context, void* activity) {
    if (ar_session_ != nullptr) return true;

    ArInstallStatus install_status;
    bool user_requested_install = !install_requested_;

    if (ArCoreApk_requestInstall(env, activity, user_requested_install, &install_status) != AR_SUCCESS)
    {
        LOGE("Please install Google Play Services for AR (ARCore).");
        return false;
    }

    if (install_status == AR_INSTALL_STATUS_INSTALL_REQUESTED)
    {
        install_requested_ = true;
        return false;
    }

    if (ArSession_create(env, context, &ar_session_) != AR_SUCCESS)
    {
        LOGE("Failed to create AR session.");
        return false;
    }

    ConfigureSession();
    ArFrame_create(ar_session_, &ar_frame_);

    return true;
}

void ARCoreManager::InitializeGL()
{
    if (ar_session_ == nullptr) return;

    glGenTextures(1, &camera_texture_id_);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, camera_texture_id_);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ArSession_setCameraTextureName(ar_session_, camera_texture_id_);
}

void ARCoreManager::ConfigureSession()
{
    ArConfig* ar_config = nullptr;
    ArConfig_create(ar_session_, &ar_config);

    ArConfig_setPlaneFindingMode(ar_session_, ar_config, AR_PLANE_FINDING_MODE_DISABLED);
    ArConfig_setDepthMode(ar_session_, ar_config, AR_DEPTH_MODE_AUTOMATIC);
    ArConfig_setInstantPlacementMode(ar_session_, ar_config, AR_INSTANT_PLACEMENT_MODE_DISABLED);

    ArSession_configure(ar_session_, ar_config);
    ArConfig_destroy(ar_config);
}

void ARCoreManager::Resume(JNIEnv* env)
{
    if (ar_session_ != nullptr)
    {
        if (ArSession_resume(ar_session_) != AR_SUCCESS)
        {
            LOGE("Failed to resume AR session.");
        }
    }
}

void ARCoreManager::Pause()
{
    if (ar_session_ != nullptr)
    {
        ArSession_pause(ar_session_);
    }
}

void ARCoreManager::Destroy()
{
    if (camera_texture_id_ != 0)
    {
        glDeleteTextures(1, &camera_texture_id_);
        camera_texture_id_ = 0;
    }

    if (ar_session_ != nullptr) {
        ArSession_destroy(ar_session_);
        ArFrame_destroy(ar_frame_);
        ar_session_ = nullptr;
        ar_frame_ = nullptr;
    }
}

void ARCoreManager::OnDisplayGeometryChanged(int display_rotation, int width, int height)
{
    if (ar_session_ != nullptr)
    {
        ArSession_setDisplayGeometry(ar_session_, display_rotation, width, height);
    }
}

void ARCoreManager::Update()
{
    if (ar_session_ == nullptr || ar_frame_ == nullptr) return;

    if (ArSession_update(ar_session_, ar_frame_) != AR_SUCCESS)
    {
        LOGE("ARCoreManager::Update ArSession_update error");
        return;
    }

    ArCamera* ar_camera;
    ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

    ArTrackingState camera_tracking_state;
    ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);

    is_tracking_ = (camera_tracking_state == AR_TRACKING_STATE_TRACKING);

    if (is_tracking_) {
        ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat_));
        ArCamera_getProjectionMatrix(ar_session_, ar_camera, 0.1f, 1000.0f, glm::value_ptr(proj_mat_));
    }

    ArCamera_release(ar_camera);
}