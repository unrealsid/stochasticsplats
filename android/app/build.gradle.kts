plugins {
    alias(libs.plugins.android.application)
}

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
                arguments += "-DSRC_ROOT=C:/Users/Sid/Documents/Visual_Studio_18/Code/StochasticSplat/src"
                arguments += "-DANDROID_VCPKG_DIR=C:/Users/Sid/Documents/Visual_Studio_18/Code/StochasticSplat/android/vcpkg_installed/arm64-android"
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
    }
}

dependencies {
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.constraintlayout)
    implementation(libs.androidx.core.ktx)
    implementation(libs.material)
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