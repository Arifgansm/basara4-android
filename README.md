# Basara 4 Android

**Sengoku Basara 4 Sumeragi · Android Native Port**

Port native PS4 game ke Android menggunakan FEXCore JIT + shadPS4 GPU core.

---

## 🚀 Quick Start

### Build APK (dengan Android Studio)

1. **Clone repository**
   ```bash
   git clone https://github.com/[YOUR-USERNAME]/basara4_android.git
   cd basara4_android
   git submodule update --init --recursive
   ```

2. **Buka di Android Studio**
   - File → Open → Pilih folder project
   - Tunggu Gradle sync selesai

3. **Konfigurasi SDK**
   - Buat file `local.properties`:
     ```
     sdk.dir=/path/to/Android/Sdk
     ndk.dir=/path/to/Android/Sdk/ndk/android-ndk-r26b
     cmake.dir=/path/to/Android/Sdk/cmake/3.22.1
     ```

4. **Build APK**
   - Build → Build Bundle(s) / APK(s) → Build APK(s)
   - Atau:
     ```bash
     ./gradlew assembleDebug
     ```

5. **Install & Run**
   ```bash
   adb install launcher/build/outputs/apk/debug/launcher-debug.apk
   ```

---

## 📋 Requirements

### Build (Host PC)
- Windows 10/11, macOS, atau Linux
- Android Studio dengan SDK 34
- Android NDK r26+
- CMake 3.22+
- Java 21+
- ~50GB storage

### Run (Android Device)
- ARM64 (aarch64) device
- Adreno 6xx/7xx/8xx GPU (Mali unsupported)
- Android 10+
- 8GB+ RAM
- 25GB+ storage untuk game dump

---

## 🎮 Cara Bermain

1. **Dump game dari PS4 Anda**
   - Game ID: CUSA01159 (Sengoku Basara 4 Sumeragi)
   - Letakkan di: `/sdcard/basara4/game/CUSA01159/`
   - Harus ada: `eboot.bin`, `sce_module/`, `sce_sys/`

2. **Buka aplikasi**
   - Launch Basara 4 Android
   - Pilih folder game
   - Konfigurasi setting (resolution, FPS, dll)
   - Tap PLAY

---

## 🛠️ Architecture

```
[PS4 eboot.bin (x86-64)]
         │
         ▼
[FEXCore JIT] ← x86_64 → ARM64
         │
         ▼
[Host CPU (ARM64)]
         │
    ┌────┴──────────────────────────────┐
    │                                   │
    ▼ (System Calls)                   ▼ (GPU Commands)
[PS4 HLE Runtime]                  [shadPS4 GPU Core]
- runtime_offline.c                - GCN → SPIR-V
- runtime_audio.c (Ngs2)           - Vulkan 1.3
- runtime_memory.c                 - Turnip Driver
- runtime_thread.c                 - Adreno GPU
         │                                   │
         ▼                                   ▼
[Android Bionic]                      [Vulkan / Turnip]
```

---

## 📂 Project Structure

| Folder | Deskripsi |
|--------|-----------|
| `launcher/` | Android app (Kotlin/Compose) |
| `src/` | PS4 runtime (C) |
| `cpu/` | FEXCore JIT integration |
| `gpu/` | shadPS4 GPU core |
| `scripts/` | Game preparation scripts |
| `third_party/` | External libraries |

---

## ⚠️ Status

| Komponen | Status |
|----------|--------|
| FEXCore CPU layer | 🚧 In development |
| shadPS4 GPU core | 🚧 In development |
| Runtime PS4 syscalls | ✅ Stub ready |
| Audio (Ngs2) | ✅ Stub ready |
| Android launcher | ✅ Ready |
| Game functionality | ❌ Requires full porting |

> **Note**: Ini adalah WIP project. Game akan berjalan setelah FEXCore dan GPU core diintegrasikan sepenuhnya.

---

## 🔗 Related Projects

- [basara4-native-pc](https://github.com/razzconnect/basara4-native-pc) - PC port (base)
- [gow3_android](https://github.com/zFitness/gow3_android) - GoW3 Android port (reference)
- [Bloodborne-Android](https://github.com/ducvd89/Bloodborne-Android) - AOT recompilation approach
- [FEX-Emu](https://github.com/FEX-Emu/FEX) - x86_64 → ARM64 JIT
- [shadPS4](https://github.com/shadps4-emu/shadPS4) - PS4 emulator GPU core

---

## 📄 License

GNU General Public License v2.0 or later.

**Legal Disclaimer**: 
- Tidak ada game files yang disertakan
- Anda harus memiliki dump legal dari PS4 Anda sendiri
- Project ini independent, tidak berafiliasi dengan Capcom atau Sony

---

## 🤝 Contributing

Pull requests welcome! Untuk kontribusi:

1. Fork repository
2. Buat feature branch (`git checkout -b feature/amazing`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing`)
5. Buat Pull Request

---

## 📧 Contact

- GitHub Issues: [Bug reports & feature requests](../../issues)
- Discord: [Razz's Server](https://discord.gg/UBZc57bBMa)

---

*Developed with ❤️ for gaming preservation*