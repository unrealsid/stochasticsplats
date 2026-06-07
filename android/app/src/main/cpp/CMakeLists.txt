cmake_minimum_required(VERSION 3.22.1)

# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("android_splatapult")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.

# Passed in from build.gradle.kts:
# -DSRC_ROOT=C:/Users/Sid/Documents/Visual_Studio_18/Code/StochasticSplat/src
# -DANDROID_VCPKG_DIR=C:/Users/Sid/Documents/Visual_Studio_18/Code/StochasticSplat/vcpkg_installed/arm64-android

add_library(android_splatapult SHARED
        "${SRC_ROOT}/core/debugrenderer.cpp"
        "${SRC_ROOT}/core/framebuffer.cpp"
        "${SRC_ROOT}/core/binaryattribute.cpp"
        "${SRC_ROOT}/core/image.cpp"
        "${SRC_ROOT}/core/log.cpp"
        "${SRC_ROOT}/core/program.cpp"
        "${SRC_ROOT}/core/texture.cpp"
        "${SRC_ROOT}/core/util.cpp"
        "${SRC_ROOT}/core/vertexbuffer.cpp"
        "${SRC_ROOT}/core/textrenderer.cpp"
        "${SRC_ROOT}/app.cpp"
        "${SRC_ROOT}/android_main.cpp"
        "${SRC_ROOT}/camerapathrenderer.cpp"
        "${SRC_ROOT}/camerasconfig.cpp"
        "${SRC_ROOT}/flycam.cpp"
        "${SRC_ROOT}/gaussiancloud.cpp"
        "${SRC_ROOT}/magiccarpet.cpp"
        "${SRC_ROOT}/ply.cpp"
        "${SRC_ROOT}/pointcloud.cpp"
        "${SRC_ROOT}/pointrenderer.cpp"
        "${SRC_ROOT}/splatrenderer.cpp"
        "${SRC_ROOT}/vrconfig.cpp"
        native-lib.cpp
        utils/imu_sensor.cpp
        utils/ar_core_manager.cpp
)

target_include_directories(android_splatapult PRIVATE
        "${SRC_ROOT}"
        "${ANDROID_VCPKG_DIR}/include"
)

add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
        "${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so"
        INTERFACE_INCLUDE_DIRECTORIES "${ARCORE_INCLUDE}"
)

# Android system libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(GLESv3-lib GLESv3)
find_library(GLESv2-lib GLESv2)

file(GLOB VCPKG_LIBS "${ANDROID_VCPKG_DIR}/lib/*.a")

target_link_libraries(android_splatapult PRIVATE
        ${log-lib}
        ${android-lib}
        ${EGL-lib}
        ${GLESv3-lib}
        ${GLESv2-lib}
        ${VCPKG_LIBS}
        arcore
)