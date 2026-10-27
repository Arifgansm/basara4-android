package com.basara4.android.nativeinit

class NativeLib {
    
    companion object {
        init {
            System.loadLibrary("basara4runtime")
        }
        
        @JvmStatic
        external fun initLogging()
        
        @JvmStatic
        external fun onAppForeground()
        
        @JvmStatic
        external fun onAppBackground()
        
        @JvmStatic
        external fun prepareGameImage(gamePath: String): Boolean
        
        @JvmStatic
        external fun launchGame(gamePath: String): Boolean
        
        @JvmStatic
        external fun setGamepadState(
            padIndex: Int,
            buttons: ByteArray,
            lstickX: Short, lstickY: Short,
            rstickX: Short, rstickY: Short,
            ltrigger: Short, rtrigger: Short
        )
        
        @JvmStatic
        external fun getGamepadState(padIndex: Int): IntArray?
    }
}