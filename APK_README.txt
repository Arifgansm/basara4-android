===================================================
  Basara 4 Android APK - Build Complete!
===================================================

APK Lokasi: ~/basara4_android/build/basara4-android.apk
Ukuran: ~16KB (stub launcher)

--- CARA INSTALL ---
1. Pindahkan APK ke HP:
   cp ~/basara4_android/build/basara4-android.apk /sdcard/Download/

2. Install via Termux:
   termux-open ~/basara4_android/build/basara4-android.apk
   
   Atau via ADB:
   adb install ~/basara4_android/build/basara4-android.apk

--- INFO PENTING ---
APK ini adalah STUB/DEVELOPMENT BUILD.
Berfungsi sebagai launcher UI saja.

Untuk menjalankan game, diperlukan:
1. FEXCore library (x86_64 -> ARM64 JIT)
2. shadPS4 GPU core (Vulkan/Turnip)
3. Game dump dari PS4 Anda (CUSA01159)

Libraries native harus di-compile di PC dan dicopy ke device.

--- STRUKTUR PROJECT ---
~/basara4_android/
├── src/                    # Java source code
├── launcher/              # Android project (Gradle)
├── cpu/                   # FEXCore integration (stub)
├── src/                   # PS4 runtime (C stubs)
├── scripts/               # Game preparation scripts
├── build_simple_apk.sh    # Build script
└── BUILD.md               # Panduan lengkap

===================================================
