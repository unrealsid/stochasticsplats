package com.the_render_box.android_splatapult

import android.app.Activity
import android.content.Context
import android.os.Debug
import androidx.annotation.Keep
import com.the_render_box.android_splatapult.utils.CameraPermissionHelper

class JniInterface {
    companion object {
        //Performs init setup when a surface is created
        @JvmStatic
        external fun onSurfaceCreated( glContext : Long, activity : Activity )

        @JvmStatic
        external fun onDrawFrame(glContext : Long)

        @JvmStatic
        external fun setAssetManager(assetManager: Any)

        @JvmStatic
        external fun setExternalDataPath(path: String)
    }
}