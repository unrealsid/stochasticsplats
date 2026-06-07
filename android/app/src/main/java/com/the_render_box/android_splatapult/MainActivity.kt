package com.the_render_box.android_splatapult

import android.content.pm.PackageManager
import android.opengl.EGL14
import android.opengl.EGLContext
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.Debug
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.snackbar.Snackbar
import com.the_render_box.android_splatapult.JniInterface.Companion.setCameraAccess
import com.the_render_box.android_splatapult.databinding.ActivityMainBinding
import com.the_render_box.android_splatapult.utils.CameraPermissionHelper
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Minimal MainActivity used as a debug bridge.
 * It loads the native library to help Android Studio attach the C++ debugger.
 */
class MainActivity : AppCompatActivity() , GLSurfaceView.Renderer {

    val TAG: String = MainActivity::class.java.getSimpleName()

    private var snackbar: Snackbar? = null
    private var surfaceView: GLSurfaceView? = null

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.surfaceView.apply {
            // Red: 8, Green: 8, Blue: 8, Alpha: 8, Depth: 24, Stencil: 0
            setEGLConfigChooser(8, 8, 8, 8, 24, 0)

            setEGLContextClientVersion(3)
        }

        binding.surfaceView.setRenderer(this)
        binding.surfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
    }

    //Called when a permission is triggered
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        // 0 is the CAMERA_PERMISSION_CODE from your CameraPermissionHelper
        if (requestCode == 0)
        {
            setCameraAccess(true)
        }
    }

    override fun onResume() {
        super.onResume()

        //ARCore requires camera permissions to operate. If we did not yet obtain runtime
        //permission on Android M and above, now is a good time to ask the user for it.
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            CameraPermissionHelper.requestCameraPermission(this)
            return
        }

        try {
            //JniInterface.onResume(nativeApplication, applicationContext, this)
        } catch (e: Exception) {
            Log.e(TAG, "Exception creating session", e)

            // Kotlin exceptions return a nullable message, so we provide a safe fallback
            return
        }
        }

    override fun onDrawFrame(gl: GL10?) {
        val currentContext: EGLContext = EGL14.eglGetCurrentContext()
        JniInterface.onDrawFrame(currentContext.nativeHandle)
    }

    override fun onSurfaceChanged(
        gl: GL10?,
        width: Int,
        height: Int
    ) {

    }

    override fun onSurfaceCreated(
        gl: GL10?,
        config: EGLConfig?
    ) {
        val currentContext: EGLContext = EGL14.eglGetCurrentContext()
        val externalPath = getExternalFilesDir(null)?.absolutePath ?: ""
        
        JniInterface.setAssetManager(assets)
        JniInterface.setExternalDataPath(externalPath)
        JniInterface.onSurfaceCreated(currentContext.nativeHandle, this)
    }

    companion object {
        init {
            System.loadLibrary("android_splatapult")
        }
    }
}
