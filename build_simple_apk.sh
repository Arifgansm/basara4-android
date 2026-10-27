#!/bin/bash
# Minimal APK builder - works WITHOUT Android SDK/NDK
# Creates a functional launcher stub that runs on Android

set -e
PREFIX=/data/data/com.termux/files/usr
PLATFORM_DIR=~/Android/platforms/android-34-2
ANDROID_JAR="$PLATFORM_DIR/android.jar"
AAPT2=$PREFIX/bin/aapt2
D8=$PREFIX/bin/d8
APKSIGNER=$PREFIX/bin/apksigner
JAVAC=$PREFIX/bin/javac

# Check if Java is available
if ! command -v java &> /dev/null; then
    echo "ERROR: Java not found. Please install Java."
    exit 1
fi

# Check if javac exists
if [ ! -f "$JAVAC" ]; then
    echo "ERROR: javac not found at $JAVAC"
    exit 1
fi

# Create build directory
BUILD_DIR=~/basara4_android/build/
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR

echo "=== Building Basara 4 Android APK ==="
echo "Note: This creates a stub APK that runs but cannot start the game."
echo "The native game requires FEXCore + shadPS4 GPU core to be built on PC."
echo ""

# 1. Compile Java sources
echo "Step 1: Compiling Java sources..."
$JAVAC \
    -source 8 -target 8 \
    -bootclasspath "$ANDROID_JAR" \
    -d $BUILD_DIR/classes \
    ~/basara4_android/src/com/basara4/android/*.java

echo "Java compiled successfully"

# 2. Create resources
echo "Step 2: Creating resources..."
mkdir -p $BUILD_DIR/res/values
cat > $BUILD_DIR/res/values/strings.xml <<'XML'
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">Basara 4 Android</string>
</resources>
XML

# 3. Create manifest (already exists at ~/basara4_android/AndroidManifest.xml)
echo "Step 3: Preparing manifest..."
cp ~/basara4_android/AndroidManifest.xml $BUILD_DIR/

# 4. Build APK with aapt2
echo "Step 4: Building APK..."
mkdir -p $BUILD_DIR/AndroidManifest
cp ~/basara4_android/AndroidManifest.xml $BUILD_DIR/AndroidManifest/AndroidManifest.xml

# Use aapt2 to compile and link
$AAPT2 compile --dir $BUILD_DIR/res -o $BUILD_DIR/resources.zip
$AAPT2 link \
    -o $BUILD_DIR/basara4-android-unsigned.apk \
    -I $PLATFORM_DIR/android.jar \
    --manifest $BUILD_DIR/AndroidManifest/AndroidManifest.xml \
    --min-sdk-version 29 \
    --target-sdk-version 34 \
    --version-code 1 \
    --version-name "1.0.0-dev" \
    $BUILD_DIR/resources.zip

# 5. Convert to DEX
echo "Step 5: Converting to DEX..."
mkdir -p $BUILD_DIR/dex
$D8 \
    --release \
    --min-api 29 \
    --lib $PLATFORM_DIR/android.jar \
    --output $BUILD_DIR/dex \
    $(find $BUILD_DIR/classes -name "*.class")

# 6. Add DEX to APK
echo "Step 6: Adding DEX to APK..."
cd $BUILD_DIR/dex
zip -q $BUILD_DIR/basara4-android-unsigned.apk classes.dex
cd -

# 7. Sign APK
echo "Step 7: Signing APK..."
# Generate debug keystore if not exists
KEYSTORE=~/basara4_android/debug.keystore
if [ ! -f $KEYSTORE ]; then
    echo "Generating debug keystore..."
    keytool -genkeypair -v \
        -keystore $KEYSTORE \
        -alias basara4debug \
        -storepass basara4 \
        -keypass basara4 \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Basara4 Debug,O=Basara4,C=US"
fi

$APKSIGNER sign \
    --ks $KEYSTORE \
    --ks-pass pass:basara4 \
    --ks-key-alias basara4debug \
    --key-pass pass:basara4 \
    --out $BUILD_DIR/basara4-android.apk \
    $BUILD_DIR/basara4-android-unsigned.apk

echo ""
echo "=== Build Complete! ==="
echo "APK: $BUILD_DIR/basara4-android.apk"
echo "Size: $(du -sh $BUILD_DIR/basara4-android.apk | cut -f1)"
echo ""
echo "Install with:"
echo "  adb install $BUILD_DIR/basara4-android.apk"
echo "or"
echo "  termux-open-app com.basara4.android"
echo ""
echo "Note: This APK is a development stub that shows the launcher UI."
echo "For full game functionality, you need to build the native libraries."
echo "See BUILD.md for detailed instructions."