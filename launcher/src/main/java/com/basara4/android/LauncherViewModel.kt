package com.basara4.android

import android.content.Context
import android.widget.Toast
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class LauncherViewModel : ViewModel() {
    
    var gamePath by mutableStateOf<String?>(null)
        private set
    var isPreparing by mutableStateOf(false)
        private set
    var error by mutableStateOf<String?>(null)
        private set
    var isLoading by mutableStateOf(false)
        private set
    
    fun pickGameFolder() {
        // In real app, use ActivityResultContracts.OpenDocumentTree
        // For dev stub, just simulate
        isLoading = true
        launch {
            delay(500)
            gamePath = "Not implemented - stub build"
            isLoading = false
        }
    }
    
    fun launchGame() {
        if (gamePath == null) {
            error = "No game folder selected (stub)"
            return
        }
        
        isPreparing = true
        error = null
        
        // Simulate preparation
        launch {
            try {
                delay(2000)
            } finally {
                isPreparing = false
                error = "Native libraries required - build full project on PC"
            }
        }
    }
}