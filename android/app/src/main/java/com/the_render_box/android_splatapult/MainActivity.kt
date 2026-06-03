package com.the_render_box.android_splatapult

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.the_render_box.android_splatapult.databinding.ActivityMainBinding

/**
 * Minimal MainActivity used as a debug bridge.
 * It loads the native library to help Android Studio attach the C++ debugger.
 */
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Force a call into C++ to trigger debugger attachment
        initNative()
    }

    external fun initNative()

    companion object {
        init {
            System.loadLibrary("android_splatapult")
        }
    }
}
