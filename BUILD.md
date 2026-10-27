# Basara 4 Android - Build Guide

## Project Overview

**Basara 4 Android** adalah port native PS4 game Sengoku Basara 4 Sumeragi (CUSA01159) ke Android.

### Stack Teknologi
- **CPU**: FEXCore JIT (x86_64 → ARM64)
- **GPU**: shadPS4 video core (GCN → Vulkan via Turnip)
- **Runtime**: PS4 system libraries reimplemented in C
- **Audio**: Ngs2 software engine + LibAtrac9 decoder
- **Launcher**: Kotlin/Compose Android app

---

## Build APK

### Metode 1: Android Studio (Recommended)

1. **Clone project**
   ```bash
   git clone https://github.com/razzconnect/basara4-android.git
   cd basara4-android
   ```

2. **Open di Android Studio**
   - File → Open → Pilih folder project
   - Tunggu Gradle sync

3. **Konfigurasi SDK/NDK**
   - Edit `local.properties`:
     ```
     sdk.dir=/path/to/Android/Sdk
     ndk.dir=/path/to/Android/Sdk/ndk/26.3.11579264
     cmake.dir=/path/to/Android/Sdk/cmake/3.22.1
     ```

4. **Build**
   - Menu: Build → Make Project
   - Atau command line:
     ```bash
     ./gradlew assembleDebug
     ```

5. **APK Output**
   - Path: `launcher/build/outputs/apk/debug/launcher-debug.apk`

---

### Metode 2: Command Line (Termux/PC)

```bash
cd ~/basara4_android

# Install dependencies
pkg install gradle openjdk-21 kotlin  # Termux only

# Set environment
export JAVA_HOME=$PREFIX/lib/jvm/java-21-openjdk
export ANDROID_HOME=~/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/android-ndk-r29

# Build
./gradlew assembleDebug
```

---

## Build Native Libraries (Advanced)

Native libraries memerlukan host x86_64 untuk cross-compile FEXCore:

```bash
# Di host x86_64 (Linux/Windows)
cd ~/basara4_android

# Clone FEXCore submodule
git submodule update --init --recursive third_party/FEX

# Build dengan Android NDK
./build_android.sh --ndk /path/to/android-ndk
```

---

## Game Files

**WAJIB**: Anda harus memiliki dump game dari PS4 Anda sendiri.

Struktur folder game:
```
game/
└── CUSA01159/
    ├── eboot.bin
    ├── sce_module/
    └── sce_sys/
```

Letakkan di direktori game atau pilih via launcher.

---

## System Requirements

### Untuk Build (PC/Host)
- **OS**: Linux/Windows/macOS x86_64
- **RAM**: 16GB minimum
- **Storage**: 50GB+
- **Android SDK**: API 34+
- **Android NDK**: r26+
- **CMake**: 3.22+
- **Java**: 21+

### Untuk Running (Android Device)
- **CPU**: ARM64 (aarch64)
- **GPU**: Adreno 6xx/7xx/8xx (Turnip driver)
- **RAM**: 8GB+
- **Storage**: 25GB+ untuk game dump
- **Android**: 10+ (API 29+)
- **Game dump**: CUSA01159 dengan update terbaru

---

## Troubleshooting

### AAPT2 Error: Unsupported attributes
```
# Ini masalah kompatibilitas aapt2 lama
# Solusi: Gunakan build-tools 34+
```

### FEXCore Build Failed
```
# FEXCore hanya support build di host x86_64
# Solusi: Build di PC, copy .so ke project
```

### APK Install Failed
```
# Gunakan adb install atau termux-open
adb install launcher/build/outputs/apk/debug/launcher-debug.apk
```

---

## Struktur Project

```
basara4_android/
├── launcher/              # Android app module
│   ├── build.gradle       # Gradle build config
│   └── src/main/
│       ├── java/com/basara4/android/
│       │   ├── MainActivity.kt      # Launcher UI
│       │   ├── LauncherViewModel.kt
│       │   └── nativeinit/NativeLib.kt
│       ├── jni/
│       │   ├── CMakeLists.txt
│       │   └── native_lib.cpp       # JNI bridge
│       └── res/
│           └── ... (resources)
├── src/                   # Runtime layer (C)
│   ├── runtime.c          # Main runtime
│   ├── runtime_offline.c  # PS4 syscalls (~130)
│   ├── runtime_audio.c    # Ngs2 audio
│   └── runtime_pad.c      # Gamepad input
├── cpu/                   # FEXCore integration
│   ├── guest_cpu.h
│   └── guest_cpu_fex.cpp  # x86_64→ARM64 JIT
├── gpu/                   # GPU layer (TODO)
├── scripts/               # Game preparation scripts
├── build.gradle           # Root build file
└── settings.gradle        # Project settings
```

---

## Roadmap

- [ ] FEXCore ARM64 cross-compile
- [ ] shadPS4 GPU core porting
- [ ] Game-specific patches (FPS unlock)
- [ ] Touch/virtual controller support
- [ ] Performance optimization
- [ ] Real device testing

---

## License

GPL-2.0+ (see LICENSE)

**Disclaimer**: Proyek ini tidak menyertakan game files. Anda harus memiliki dump legal dari PS4 Anda sendiri.