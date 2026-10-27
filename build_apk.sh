#!/bin/bash
# Simple APK builder - works WITHOUT Android SDK/NDK
# Builds a minimal APK from source using javac + aapt2 + d8
# This produces a launcher stub that runs but cannot start the game.

set -e

PREFIX=/data/data/com.termux/files/usr
AAPT2="$PREFIX/bin/aapt2"
D8="$PREFIX/bin/d8"
APKSIGNER="$PREFIX/bin/apksigner"
JAVAC="$PREFIX/bin/javac"
JAVA_HOME="$PREFIX/lib/jvm/java-21-openjdk"

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAUNCHER_SRC="$PROJECT_DIR/launcher/src/main"
BUILD_DIR="$PROJECT_DIR/build/apk"
APK_NAME="basara4-android-stub"

mkdir -p "$BUILD_DIR"

echo "=== Basara 4 Android - APK Builder ==="
echo "Note: This builds a STUB APK. Game functionality requires:"
echo "  - FEXCore (x86_64->ARM64 JIT)"
echo "  - shadPS4 GPU core"
echo "  - Your game dump (CUSA01159)"
echo ""

# 1. Generate Java/Kotlin sources
echo "Step 1: Generating Java bridge class..."
mkdir -p "$BUILD_DIR/gen"
cat > "$BUILD_DIR/gen/com/basara4/android/NativeBridge.java" <<'JAVA'
package com.basara4.android;

/**
 * Native bridge - stub implementation.
 * Real implementation loads FEXCore and shadPS4 GPU libraries.
 */
public class NativeBridge {
    public static boolean isNativeAvailable() {
        // Would check for libbasara4cpu.so and libbasara4gpu.so
        return false;
    }
}
JAVA

# 2. Create AndroidManifest (from source)
echo "Step 2: Processing AndroidManifest..."
MANIFEST_SRC="$LAUNCHER_SRC/AndroidManifest.xml"

# 3. Compile resources
echo "Step 3: Compiling resources..."
RES_DIR="$LAUNCHER_SRC/res"
if [ -d "$RES_DIR" ]; then
    "$AAPT2" compile --dir "$RES_DIR" -o "$BUILD_DIR/res.apk" 2>/dev/null || {
        echo "WARNING: Resource compilation skipped (aapt2 issues)"
        # Create minimal res.apk
        touch "$BUILD_DIR/res.apk"
    }
fi

# 4. Create minimal Java stub (since Kotlin requires full toolchain)
echo "Step 4: Creating Java stub for APK..."
mkdir -p "$BUILD_DIR/java/com/basara4/android"
cat > "$BUILD_DIR/java/com/basara4/android/MainActivity.java" <<'JAVA'
package com.basara4.android;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * Basara 4 Android - Main Activity
 * Launcher screen - stub version for APK testing
 */
public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(buildUI());
    }
    
    private View buildUI() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(32, 32, 32, 32);
        
        // Title
        TextView title = new TextView(this);
        title.setText("Sengoku Basara 4\nSumeragi");
        title.setTextSize(28);
        title.setGravity(android.view.Gravity.CENTER);
        root.addView(title, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Subtitle
        TextView subtitle = new TextView(this);
        subtitle.setText("Android Native Port - Development Stub");
        subtitle.setTextSize(14);
        subtitle.setGravity(android.view.Gravity.CENTER);
        subtitle.setPadding(0, 16, 0, 16);
        root.addView(subtitle, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Status
        TextView status = new TextView(this);
        status.setText("Status: STUB BUILD\n\n" +
                      "• No FEXCore JIT integration\n" +
                      "• No shadPS4 GPU core\n" +
                      "• No game dump loaded\n\n" +
                      "For full functionality:\n" +
                      "1. Build with FEXCore + shadPS4\n" +
                      "2. Place game dump in /sdcard/basara4/\n" +
                      "3. Tap PLAY button\n\n" +
                      "GPU: Adreno 6xx/7xx/8xx recommended\n" +
                      "Storage: ~25GB for game files");
        status.setTextSize(12);
        status.setGravity(android.view.Gravity.CENTER);
        root.addView(status, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 0, 1.0f));
        
        // Button
        android.widget.Button btn = new android.widget.Button(this);
        btn.setText("PLAY (Not Available)");
        btn.setAllCaps(false);
        btn.setAlpha(0.5f);
        btn.setEnabled(false);
        root.addView(btn, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        return root;
    }
}
JAVA

cat > "$BUILD_DIR/java/com/basara4/android/StubApplication.java" <<'JAVA'
package com.basara4.android;

import android.app.Application;

public class StubApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        // Would load native libraries here
        // System.loadLibrary("basara4cpu");
        // System.loadLibrary("basara4gpu");
        // System.loadLibrary("basara4runtime");
    }
}
JAVA

# 5. Compile Java
echo "Step 5: Compiling Java sources..."
PLATFORM_DIR="/data/data/com.termux/files/home/Android/platforms/android-34"
BOOT_CLASSPATH="$PLATFORM_DIR/android.jar"
if [ ! -f "$BOOT_CLASSPATH" ]; then
    # Use system platform if available
    for d in /system/framework /data/data/com.termux/files/usr/lib; do
        [ -f "$d/android.jar" ] && BOOT_CLASSPATH="$d/android.jar" && break
    done
fi

if [ -n "$BOOT_CLASSPATH" ] && [ -f "$BOOT_CLASSPATH" ]; then
    "$JAVAC" \
        -cp "$BOOT_CLASSPATH" \
        -d "$BUILD_DIR/classes" \
        "$BUILD_DIR/java/com/basara4/android/"*.java
    echo "Java compiled successfully"
else
    echo "WARNING: android.jar not found - using minimal compilation"
    # Fallback: create stub class files that won't reference Android APIs
    echo "Cannot fully compile without android.jar"
fi

# 6. Build APK
echo "Step 6: Building APK..."

# Generate minimal resources.arsc if needed
RES_AAR="$BUILD_DIR/res.apk"
[ ! -f "$RES_AAR" ] && touch "$RES_AAR"

# Use aapt2 to link
LINK_OUT="$BUILD_DIR/linked"
mkdir -p "$LINK_OUT"

# Create a minimal manifest for aapt2
cat > "$LINK_OUT/AndroidManifest.xml" <<'XML'
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.basara4.android"
    android:versionCode="1"
    android:versionName="1.0.0-dev">
    
    <uses-sdk android:minSdkVersion="29" android:targetSdkVersion="34" />
    
    <application
        android:name=".StubApplication"
        android:label="Basara 4 Android"
        android:theme="@android:style/Theme.Material.NoActionBar">
        
        <activity
            android:name=".MainActivity"
            android:exported="true"
            android:screenOrientation="landscape">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
        
    </application>
</manifest>
XML

# Try aapt2 link
if command -v "$AAPT2" >/dev/null 2>&1; then
    "$AAPT2" link \
        -o "$LINK_OUT/app.apt" \
        -I "$PLATFORM_DIR/android.jar" 2>/dev/null \
        --manifest "$LINK_OUT/AndroidManifest.xml" \
        --min-sdk-version 29 \
        --target-sdk-version 34 \
        --version-code 1 \
        --version-name "1.0.0-dev" \
        "$RES_AAR" 2>&1 || echo "WARNING: aapt2 link had issues"
fi

# 7. D8 (DEX conversion)
echo "Step 7: Converting to DEX..."
if [ -d "$BUILD_DIR/classes" ]; then
    "$D8" \
        --release \
        --min-api 29 \
        --output "$BUILD_DIR/dex/" \
        "$BUILD_DIR/classes/com/basara4/android/"*.class 2>&1 || \
    echo "WARNING: DEX conversion had issues"
fi

# 8. Final APK assembly
echo "Step 8: Assembling final APK..."

APK_OUT="$PROJECT_DIR/${APK_NAME}-$(date +%Y%m%d_%H%M).apk"

# Create APK (simple zip with manifest)
cd "$BUILD_DIR"
rm -f "$APK_OUT"
zip -q "$APK_OUT" "dex/" 2>/dev/null || {
    # Fallback: just create a marker APK
    echo "WARNING: Full APK assembly not available"
    echo "Use Android Studio to build the final APK"
}

# 9. Sign APK
echo "Step 9: Signing APK..."
KEYSTORE="$PROJECT_DIR/debug.keystore"
if [ ! -f "$KEYSTORE" ]; then
    keytool -genkeypair -v \
        -keystore "$KEYSTORE" \
        -alias androiddebugkey \
        -storepass android \
        -keypass android \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US" 2>/dev/null || {
            echo "WARNING: Keygen failed"
        }
fi

if command -v "$APKSIGNER" >/dev/null 2>&1 && [ -f "$APK_OUT" ]; then
    "$APKSIGNER" sign \
        --ks "$KEYSTORE" \
        --ks-pass "password:android" \
        --ks-key-alias "androiddebugkey" \
        --key-pass "password:android" \
        "$APK_OUT" 2>&1 || \
    echo "APK created unsigned - sign with keytool/apksigner"
fi

echo ""
echo "=== Build Complete ==="
if [ -f "$APK_OUT" ]; then
    echo "APK: $APK_OUT"
    echo "Size: $(du -sh "$APK_OUT" | cut -f1)"
    echo ""
    echo "Install with:"
    echo "  adb install $APK_OUT"
    echo "or"
    echo "  termux-open-app com.basara4.android"
else
    echo "APK assembly incomplete - use Android Studio to build:"
    echo "  cd ~/basara4_android/launcher"
    echo "  ./gradlew assembleDebug"
fi