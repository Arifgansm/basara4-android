/*
 * Basara 4 Android - App Module build.gradle.kts
 * This is a Kotlin DSL Gradle build file
 */

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.basara4.android"
    compileSdk = 34
    
    defaultConfig {
        applicationId = "com.basara4.android"
        minSdk = 29
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0-dev"
        
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        
        // NDK configuration
        externalNativeBuild {
            cmake {
                cppFlags.add("-std=c++20")
                cppFlags.add("-frtti")
                cppFlags.add("-fexceptions")
                arguments.add("-DANDROID_STL=c++_shared")
            }
        }
        
        // ABI filters - we only support arm64-v8a for this port
        ndk {
            abiFilters.add("arm64-v8a")
        }
    }
    
    buildTypes {
        debug {
            isDebuggable = true
            isMinifyEnabled = false
            isShrinkResources = false
            
            matchingFallbacks.add("release")
        }
        
        release {
            isMinifyEnabled = false
            isShrinkResources = false
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
    
    kotlinOptions {
        jvmTarget = "11"
        freeCompilerArgs.add("-Xopt-in=kotlin.RequiresOptIn")
    }
    
    // Compose setup
    composeOptions {
        kotlinCompilerExtensionVersion = "1.5.13"
    }
    
    // Packaging options for native libraries
    packagingOptions {
        jniLibs {
            useLegacyPackaging = true
        }
    }
    
    // External native build
    externalNativeBuild {
        cmake {
            path("src/main/jni/CMakeLists.txt")
            version("3.22.1")
        }
    }
    
    // Resources
    resources {
        // Enable non-translatable strings
    }
}

dependencies {
    // Core Android
    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.activity:activity-compose:1.8.2")
    
    // Material 3
    implementation("androidx.compose.material3:material3:1.1.2")
    
    // Compose
    implementation(platform("androidx.compose:compose-bom:2024.02.00"))
    implementation("androidx.compose.ui:ui:1.5.4")
    implementation("androidx.compose.ui:ui-graphics:1.5.4")
    implementation("androidx.compose.ui:ui-tooling-preview:1.5.4")
    implementation("androidx.compose.material3:material3:1.1.2")
    implementation("androidx.compose.runtime:runtime-livedata:1.5.4")
    implementation("androidx.lifecycle:lifecycle-runtime-compose:2.7.0")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.7.0")
    
    // Lifecycle
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.7.0")
    implementation("androidx.lifecycle:lifecycle-livedata-ktx:2.7.0")
    
    // Kotlin Coroutines
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
    
    // Testing
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
    androidTestImplementation(platform("androidx.compose:compose-bom:2024.02.00"))
    androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.5.4")
    debugImplementation("androidx.compose.ui:ui-tooling:1.5.4")
    debugImplementation("androidx.compose.ui:ui-test-manifest:1.5.4")
}