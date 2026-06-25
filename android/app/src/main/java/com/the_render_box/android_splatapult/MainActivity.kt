package com.the_render_box.android_splatapult

import android.content.pm.PackageManager
import android.opengl.EGL14
import android.opengl.EGLContext
import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.snackbar.Snackbar
import com.the_render_box.android_splatapult.databinding.ActivityMainBinding
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

        supportActionBar?.hide()

        binding.surfaceView.apply {
            // Red: 8, Green: 8, Blue: 8, Alpha: 8, Depth: 24, Stencil: 0
            setEGLConfigChooser(8, 8, 8, 8, 24, 0)

            setEGLContextClientVersion(3)

            setOnClickListener {
                JniInterface.onTap()
            }
        }

        binding.surfaceView.setRenderer(this)
        binding.surfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY

        hideSystemUI()
    }

    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false)
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUI()
        }
    }

    override fun onResume() {
        super.onResume()
        binding.surfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        binding.surfaceView.onPause()
        JniInterface.onPause()
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
        val displayRotation = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            display?.rotation ?: 0
        } else {
            @Suppress("DEPRECATION")
            windowManager.defaultDisplay.rotation
        }
        JniInterface.onSurfaceChanged(width, height, displayRotation)
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
