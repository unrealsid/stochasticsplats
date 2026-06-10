#pragma once

#include <jni.h>
#include <GLES3/gl3.h>
#include "glm/glm.hpp"
#include "arcore_c_api.h"

class ARCoreManager
{
public:
    ARCoreManager();
    ~ARCoreManager();

    bool Initialize(JNIEnv* env, void* context, void* activity);
    void InitializeGL();
    void Resume(JNIEnv* env);
    void Pause();
    void Destroy();

    void OnDisplayGeometryChanged(int display_rotation, int width, int height);
    void Update();
    void SetTargetFps(int target_fps);

    [[nodiscard]] glm::mat4 GetViewMatrix() const { return view_mat_; }
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const { return proj_mat_; }
    [[nodiscard]] bool IsTracking() const { return is_tracking_; }

private:
    void ConfigureSession();

    ArSession* ar_session_ = nullptr;
    ArFrame* ar_frame_ = nullptr;
    GLuint camera_texture_id_ = 0;

    bool install_requested_ = false;
    bool is_tracking_ = false;
    int target_fps_ = 30;

    glm::mat4 view_mat_ = glm::mat4(1.0f);
    glm::mat4 proj_mat_ = glm::mat4(1.0f);
};
