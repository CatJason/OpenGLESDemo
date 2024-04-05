package com.jason.openglesdemo

import android.view.View
import com.google.androidgamesdk.GameActivity

class MainActivity : FullScreenGameActivity() {
    companion object {
        init {
            System.loadLibrary("openglesdemo")
        }
    }
}