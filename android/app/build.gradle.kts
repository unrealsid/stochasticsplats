plugins {
    alias(libs.plugins.android.application)
}

val arcoreLibPath = layout.buildDirectory.dir("arcore-native").get().asFile.absolutePath

File(arcoreLibPath, "include").mkdirs()
File(arcoreLibPath, "jni").mkdirs()

val natives by configurations.creating

android {
    namespace = "com.the_render_box.android_splatapult"
    compileSdk {
        version = release(36) {
            minorApiLevel = 1
        }
    }

    defaultConfig {
        applicationId = "com.the_render_box.android_splatapult"
        minSdk = 35
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DCMAKE_CXX_STANDARD=17"
                arguments += "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
                arguments += "-DSRC_ROOT=${project.rootDir.absolutePath.replace("\\", "/")}/../src"
                arguments += "-DANDROID_VCPKG_DIR=${project.rootDir.absolutePath.replace("\\", "/")}/vcpkg_installed/arm64-android"
                arguments += "-DARCORE_LIBPATH=${arcoreLibPath.replace("\\", "/")}/jni"
                arguments += "-DARCORE_INCLUDE=${arcoreLibPath.replace("\\", "/")}/include"
                abiFilters += "arm64-v8a"
            }
        }
    }

    buildTypes {
        debug {
            isMinifyEnabled = false
            isDebuggable = true
            isJniDebuggable = true
        }
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildFeatures {
        viewBinding = true
        prefab = true
    }
}

dependencies {
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.constraintlayout)
    implementation(libs.androidx.core.ktx)
    implementation(libs.material)

    implementation(libs.arcore)

    //Tell the natives configuration to fetch the ARCore AAR
    natives(libs.arcore)

    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
}

// Create a task to copy the external folders into Android's native assets folder
tasks.register<Copy>("copyNativeAssets") {
    from("../../texture") { into("texture") }
    from("../../shader") { into("shader") }
    from("../../font") { into("font") }
    from("../../data") { into("data") }

    // Dump them exactly where Android natively expects them
    into("src/main/assets")
}

// Tell Gradle it must run this copy task before it builds the app
tasks.named("preBuild") {
    dependsOn("copyNativeAssets")
}

// Register the task
tasks.register("extractNativeLibraries") {
    // Always extract, this ensures the native libs are updated if the version changes.
    outputs.upToDateWhen { false }

    doFirst {
        configurations.getByName("natives").files.forEach { f ->
            copy {
                from(zipTree(f))
                into(arcoreLibPath)
                include("jni/**/*")
                include("include/**/*")
            }
        }
    }
}

// Ensure the task runs before external native builds
tasks.configureEach {
    if ((name.contains("external") || name.contains("CMake")) && !name.contains("Clean")) {
        dependsOn("extractNativeLibraries")
    }
}