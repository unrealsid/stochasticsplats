/*
    Copyright (c) 2024 Anthony J. Thibault
    This software is licensed under the MIT License. See LICENSE for more details.
*/

#include <android/native_window_jni.h> // for native window JNI
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <jni.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <android/asset_manager.h>
#include <chrono>
#include <thread>

#include "core/log.h"
#include "core/util.h"
#include "app.h"
#include <glm/gtc/matrix_transform.hpp>
#include "flycam.h"

// see ovrApp::HandleSessionStateChanges in SceneModelXr.cpp
/*
static const int CPU_LEVEL = 2;
static const int GPU_LEVEL = 3;
*/

static const char* EglErrorString(const EGLint error) {
    switch (error) {
        case EGL_SUCCESS:
            return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:
            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:
            return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:
            return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:
            return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:
            return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:
            return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:
            return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:
            return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:
            return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:
            return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:
            return "EGL_CONTEXT_LOST";
        default:
            return "unknown";
    }
}

//Globals
std::unique_ptr<App> g_app = nullptr;
jobject g_activityGlobal = nullptr;
glm::mat4 g_lastViewMat;
bool g_cameraLocked = false;

//point_cloud_truck_30k or treehill_point_cloud
std::string dataFilePath = "data/point_cloud_truck_30k.ply";

struct AppContext
{
    AppContext() : resumed(false), sessionActive(false), assMan(nullptr), alwaysCopyAssets(true) {}
    bool resumed;
    bool sessionActive;

    struct EGLInfo
    {
        EGLInfo() : majorVersion(0), minorVersion(0), display(0), config(0), context(EGL_NO_CONTEXT) {}
        EGLint majorVersion;
        EGLint minorVersion;
        EGLDisplay display;
        EGLConfig config;
        EGLContext context;
        EGLSurface windowSurface;
    };

    EGLInfo egl;
    AAssetManager* assMan;
    std::string externalDataPath;
    bool alwaysCopyAssets;

    void Clear()
    {
        resumed = false;
        sessionActive = false;
        egl.majorVersion = 0;
        egl.minorVersion = 0;
        egl.display = 0;
        egl.config = 0;
        egl.context = EGL_NO_CONTEXT;
    }

    AAssetManager* g_asset_manager;

    bool SetupAssets(AAssetManager* assetManager, const std::string& externalPath)
    {
        assert(assetManager);
        assert(externalPath != "");

        externalDataPath = externalPath + "/";
        g_asset_manager = assetManager;

        // from util.h
        SetRootPath(externalDataPath);

        Log::D("AJT: externalDataPath = \"%s\"\n", externalDataPath.c_str());

        MakeDir("texture");
        UnpackAsset("texture/carpet.png");
        UnpackAsset("texture/sphere.png");

        MakeDir("shader");
        UnpackAsset("shader/carpet_frag.glsl");
        UnpackAsset("shader/carpet_vert.glsl");
        UnpackAsset("shader/debugdraw_frag.glsl");
        UnpackAsset("shader/debugdraw_vert.glsl");
        UnpackAsset("shader/desktop_frag.glsl");
        UnpackAsset("shader/desktop_vert.glsl");
        UnpackAsset("shader/point_frag.glsl");
        UnpackAsset("shader/point_geom.glsl");
        UnpackAsset("shader/point_vert.glsl");
        UnpackAsset("shader/presort_compute.glsl");
        UnpackAsset("shader/multi_radixsort.glsl");
        UnpackAsset("shader/multi_radixsort_histograms.glsl");
        UnpackAsset("shader/single_radixsort.glsl");
        UnpackAsset("shader/splat_frag.glsl");
        UnpackAsset("shader/splat_frag_ST.glsl");
        UnpackAsset("shader/splat_geom.glsl");
        UnpackAsset("shader/splat_vert.glsl");
        UnpackAsset("shader/text_frag.glsl");
        UnpackAsset("shader/text_vert.glsl");

        UnpackAsset("shader/warp_frag.glsl");
        UnpackAsset("shader/warp_vert.glsl");

        UnpackAsset("shader/avg_vert.glsl");
        UnpackAsset("shader/avg_frag.glsl");

        UnpackAsset("shader/display_frag.glsl");

        MakeDir("font");
        UnpackAsset("font/JetBrainsMono-Medium.json");
        UnpackAsset("font/JetBrainsMono-Medium.png");

        MakeDir("data");
        MakeDir("data/sh_test");
        //UnpackAsset("data/sh_test/cameras.json");
        //UnpackAsset("data/sh_test/cfg_args");
        //UnpackAsset("data/sh_test/input.ply");
        MakeDir("data/sh_test/point_cloud");
        MakeDir("data/sh_test/point_cloud/iteration_30000");
        UnpackAsset("data/test_vr.json");
        UnpackAsset(dataFilePath);
        //UnpackAsset("data/test.ply");

        return true;
    }

    bool MakeDir(const std::string& dirFilename)
    {
        std::string filename = externalDataPath + dirFilename;
        if (mkdir(filename.c_str(), 0777) != 0)
        {
            if (errno == EEXIST)
            {
                Log::D("MakeDir \"%s\" already exists\n", dirFilename.c_str());
                // dir already exists!
                return true;
            }
            else
            {
                Log::E("mkdir failed on dir \"%s\" errno = %d\n", filename.c_str(), errno);
                return false;
            }
        }
        Log::D("MakeDir \"%s\"\n", dirFilename.c_str());

        return true;
    }

    bool UnpackAsset(const std::string& assetFilename)
    {
        std::string outputFilename = externalDataPath + assetFilename;

        struct stat sb;
        if (stat(outputFilename.c_str(), &sb) == 0)
        {
            if (!alwaysCopyAssets)
            {
                Log::D("UnpackAsset \"%s\" already exists\n", assetFilename.c_str());
                return true;
            }
        }

        AAsset *asset = AAssetManager_open(g_asset_manager, assetFilename.c_str(), AASSET_MODE_STREAMING);
        if (asset == nullptr)
        {
            Log::E("UnpackAsset \"%s\" AAssetManager_open failed!\n", assetFilename.c_str());
            return false; // Failed to open the asset
        }

        // Create buffer for reading
        const size_t BUFFER_SIZE = 2048;
        char buffer[BUFFER_SIZE];

        // Open file for writing
        FILE *outFile = fopen(outputFilename.c_str(), "w");
        if (outFile == nullptr)
        {
            Log::E("UnpackAsset \"%s\" fopen failed!\n", assetFilename.c_str());
            AAsset_close(asset);
            return false; // Failed to open the output file
        }

        // Read from assets and write to file
        int bytesRead;
        while ((bytesRead = AAsset_read(asset, buffer, BUFFER_SIZE)) > 0)
        {
            fwrite(buffer, sizeof(char), bytesRead, outFile);
        }

        // Close the asset and the output file
        AAsset_close(asset);
        fclose(outFile);

        Log::D("UnpackAsset \"%s\"\n", assetFilename.c_str());
        return true;
    }
};

int g_screenWidth = 0;
int g_screenHeight = 0;
int g_displayRotation = 0;
static std::chrono::steady_clock::time_point g_lastFrameTime;

void android_init(JNIEnv* env, jlong gl_context, jobject activity, AAssetManager* asset_manager, const std::string& externalPath)
{
    Log::SetAppName("splatapult");

    Log::D("----------------------------------------------------------------\n");
    Log::D("android_init()\n");

    if (g_activityGlobal) {
        env->DeleteGlobalRef(g_activityGlobal);
    }
    g_activityGlobal = env->NewGlobalRef(activity);

    AppContext ctx;

    if (!ctx.SetupAssets(asset_manager, externalPath))
    {
        Log::E("AppContext::SetupAssets failed!\n");
        return;
    }

    MainContext mainContext;
    mainContext.display = eglGetCurrentDisplay();

    std::string dataPath = ctx.externalDataPath + dataFilePath;
    int argc = 6;
    const char* argv[] = {"splatapult", "-v", "-d", "--render_mode", "ST", dataPath.c_str()};

    g_app = std::make_unique<App>(mainContext);

    App::ParseResult parseResult = g_app->ParseArguments(argc, argv);
    switch (parseResult)
    {
    case App::SUCCESS_RESULT:
        break;
    case App::ERROR_RESULT:
        Log::E("App::ParseArguments failed!\n");
        g_app.reset();
        return;
    case App::QUIT_RESULT:
        g_app.reset();
        return;
    }

    if (!g_app->Init())
    {
        Log::E("App::Init failed!\n");
        g_app.reset();
        return;
    }

    // TODO: DESTROY STUFF
    Log::D("Finished!\n");
}

void android_render()
{
    if (g_app == nullptr) return;

    // Lock frame rate
    int targetFps = g_app->GetTargetFps();
    if (targetFps > 0)
    {
        auto now = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::microseconds(1000000 / targetFps);
        auto elapsed = now - g_lastFrameTime;
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(frameDuration - elapsed);
            now = std::chrono::steady_clock::now();
        }
        g_lastFrameTime = now;
    }

    EGLint screenWidth = 0;
    EGLint screenHeight = 0;

    EGLDisplay display = eglGetCurrentDisplay();
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);

    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);

    static float orbitYaw = 0.0f;
    static auto lastUpdateTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
    lastUpdateTime = currentTime;
    if (dt <= 0.0f || dt > 0.1f)
    {
        dt = 1.0f / (float)(targetFps > 0 ? targetFps : 30);
    }

    if (!g_cameraLocked && g_app && g_app->GetFlyCam())
    {
        orbitYaw += dt * 0.5f;
        g_app->GetFlyCam()->Orbit(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f, glm::radians(0.0), orbitYaw);
    }

    if (!g_app->Render(dt, glm::ivec2(screenWidth, screenHeight)))
    {
        Log::E("App::Render failed!\n");
        return;
    }
}

void android_onSurfaceChanged(int width, int height, int displayRotation)
{
    g_screenWidth = width;
    g_screenHeight = height;
    g_displayRotation = displayRotation;
}

void android_onResume(JNIEnv* env)
{
}

void android_onPause()
{
}

void android_onTap()
{
    g_cameraLocked = !g_cameraLocked;
    Log::I("AR Camera %s", g_cameraLocked ? "LOCKED" : "UNLOCKED");
}


