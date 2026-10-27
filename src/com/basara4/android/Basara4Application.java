package com.basara4.android;

import android.app.Application;

public class Basara4Application extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        // Load native libraries when app starts
        // System.loadLibrary("basara4cpu");
        // System.loadLibrary("basara4gpu");
        // System.loadLibrary("basara4runtime");
    }
}