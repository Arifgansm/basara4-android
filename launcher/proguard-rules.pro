# Keep some debugging symbols for crash reports
-dontoptimize
-dontobfuscate
-keep class com.basara4.android.** { *; }
-keep class com.basara4.android.nativeinit.** { *; }

# Native methods must be preserved
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}

# Keep Kotlin metadata
-keep class kotlin.Metadata { *; }