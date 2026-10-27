package com.basara4.android

import android.app.Application
import android.content.Context
import android.os.Build
import androidx.appcompat.app.AppCompatDelegate
import androidx.lifecycle.ProcessLifecycleOwner

class Basara4Application : Application() {

    override fun onCreate() {
        super.onCreate()
        
        // Force dark mode for the Sengoku aesthetic
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)
        
        // Initialize native libraries
        System.loadLibrary("basara4cpu")
        System.loadLibrary("basara4gpu")
        System.loadLibrary("basara4runtime")
        
        // Initialize logging
        com.basara4.android.nativeinit.NativeLib.initLogging()
        
        // Setup process lifecycle
        ProcessLifecycleOwner.get().lifecycle.addObserver(object : androidx.lifecycle.DefaultLifecycleObserver() {
            override fun onStart(owner: androidx.lifecycle.LifecycleOwner) {
                com.basara4.android.nativeinit.NativeLib.onAppForeground()
            }
            override fun onStop(owner: androidx.lifecycle.LifecycleOwner) {
                com.basara4.android.nativeinit.NativeLib.onAppBackground()
            }
        })
    }
}