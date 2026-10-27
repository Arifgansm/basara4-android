# PS4 Native Game → Android Port

Use when porting a PS4 native PC port (e.g. bloodborne_pc, gow3_pc, basara4-native-pc) to Android. This covers the specific architecture: FEXCore JIT for x86_64→ARM64 translation, shadPS4 GPU core for Vulkan, and game-specific runtime HLE.

## Why This Skill Exists

PS4 uses x86_64, same as PC. A "native PC port" runs the game's x86_64 code directly via a custom runtime that reimplements PS4 system calls. To run on Android (ARM64), you need:
1. **FEXCore**: JIT translator that converts x86_64 guest code to ARM64 at runtime
2. **shadPS4 GPU core**: Translates PS4 GPU commands (GNM/GCM) to Vulkan
3. **Game-specific runtime**: Reimplements PS4 syscalls (memory, threads, audio, etc.)
4. **Android launcher**: Kotlin/Compose UI with JNI bridge

This is NOT emulation — it's a game-specific port. The same upstream code (SpartanPort/bloodborne_pc) powers all these projects.

## Procedure

### 1. Verify Project Compatibility

Check if the source project is a SpartanPort derivative:
- Look for `bloodborne_pc`, `SpartanPort`, or `gow3_pc` in README/NOTICE
- Check if `src/runtime_*.c` exists (runtime layer)
- Check if `gpu/shadps4/` exists (GPU layer)
- If yes, proceed. If no, the project uses a different architecture.

**Pitfall**: Not all PS4 ports use this architecture. Some use QEMU or RPCS3-style emulation. Verify before investing effort.

### 2. Clone Source Project

```bash
git clone --recursive <source-repo>
cd <project>
git submodule update --init --recursive
```

The project must include:
- `src/runtime_offline.c` or similar (~130 PS4 syscall stubs)
- `gpu/shadps4/` (Vulkan translation core)
- `scripts/prepare.py` (ELF parsing for game image prep)

### 3. Create Android Project Structure

```
basara4_android/
├── cpu/                    # FEXCore integration layer
│   ├── guest_cpu.h         # CPU interface header
│   ├── guest_cpu_fex.cpp   # FEXCore backend
│   ├── host_call_arm64.S   # ARM64 assembly stubs
│   └── basara4cpu.cmake    # Build config
├── src/                    # Runtime layer
│   ├── runtime.h           # Global state header
│   ├── probe.c             # Entry point / loader
│   ├── runtime.c           # Android-adapted runtime
│   ├── runtime_cpu.c       # CPU interface wrapper
│   ├── runtime_offline.c   # PS4 syscall stubs
│   ├── runtime_audio.c     # Ngs2/Atrac9 audio
│   └── runtime_pad.c       # Gamepad input
├── launcher/               # Android app
│   └── src/main/
│       ├── AndroidManifest.xml
│       ├── java/com/<pkg>/
│       │   ├── MainActivity.kt
│       │   ├── Basara4Application.kt
│       │   └── nativeinit/NativeLib.kt
│       └── jni/native_lib.cpp  # JNI bridge
├── scripts/                # Game preparation
│   ├── prepare.py
│   ├── link_libc.py
│   └── link_modules.py
├── gpu/                    # shadPS4 GPU (reference from gow3_android)
├── CMakeLists.txt          # Top-level build
└── build_android.sh        # Build script
```

### 4. Add FEXCore Submodule

```bash
cd ~/basara4_android
git init
git submodule add https://github.com/FEX-Emu/FEX.git third_party/FEX
```

**Requirement**: FEXCore must be built for Android ARM64. The build uses `cmake -DFEX_ENABLE_LTO=ON`.

### 5. Implement CPU Interface

The CPU interface (`cpu/guest_cpu.h`) defines the contract between guest (x86_64 game) and host (ARM64):

```c
void guest_cpu_init(void *(*map)(size_t size, int prot));
void guest_cpu_code(uintptr_t base, size_t size);
uintptr_t guest_cpu_host_function(void *fn);
uint64_t guest_cpu_call(uintptr_t fn, unsigned count, const uint64_t *args);
int guest_cpu_hook(uintptr_t address, size_t size, GuestHook hook);
```

Implement in `guest_cpu_fex.cpp` using FEXCore API:
- Create `FEXCore::Context`
- Register syscall handler
- Allocate stub table for host calls
- Handle guest memory regions

**Pitfall**: FEXCore expects Linux-like syscalls. Android Bionic differs. You may need to intercept and redirect certain syscalls through the runtime.

### 6. Adapt Runtime for Android

Key differences from PC runtime:
| Component | PC | Android |
|-----------|----|---------|
| Memory allocation | `mmap`/`munmap` | Same, but needs low-address mapping (<40-bit) |
| Threads | pthreads | pthreads (works in Termux) |
| File I/O | POSIX | Android scoped storage / external SD |
| Audio | WASAPI/PulseAudio | Android AudioTrack |
| Input | SDL3/gamepad | Android InputManager → JNI |
| GPU | Vulkan desktop | Vulkan + Turnip (Adreno) or SwiftShader |

**Critical adaptation**: `runtime_memory.c` must use `android_low_map()` that returns addresses below 2^40 (the game packs pointers into 40 bits).

### 7. Build GPU Layer

Reference `gow3_android/gpu/` for the Android shadPS4 adaptation:
- Cross-compile shadPS4 GPU core for ARM64
- Link against Turnip driver (Mesa Vulkan for Adreno)
- Enable VK_KHR_android_surface

**Pitfall**: Mali GPUs are NOT supported. Only Adreno (Snapdragon) with Turnip driver works.

### 8. Build Android Launcher

The launcher manages:
- Game folder selection (external storage)
- Image preparation (call `prepare.py` via JNI)
- Runtime lifecycle (foreground/background)
- Surface creation for Vulkan

**Pitfall**: The launcher must verify game dump integrity (SHA-256) before loading. Never trust unverified game files.

### 9. Build Instructions

```bash
export ANDROID_NDK=/path/to/android-ndk
./build_android.sh --target all --ndk $ANDROID_NDK
```

Then build APK:
```bash
cd launcher
./gradlew assembleDebug
adb install launcher/build/outputs/apk/debug/*.apk
```

## Known Pitfalls

1. **FEXCore is still being validated on Android**: Performance numbers not stable. Expect stuttering during JIT compilation of new code regions.

2. **Turnip driver compatibility**: Requires Snapdragon (Adreno 6xx+). Samsung/MediaTek Mali GPUs not supported.

3. **Game dump required**: No game files provided. User must have legal dump from their PS4 (CUSA ID).

4. **Signatures are game-specific**: `signature.h` and `patches/` contain game-specific byte patterns. These must be adapted for each game.

5. **Build environment**: Requires x86_64 host for cross-compilation. Cannot build on device.

## References

- `references/fexcore-android-build.md` — FEXCore Android build notes
- `references/spartanport-architecture.md` — Architecture comparison
- `references/game-specific-adaptations.md` — Checklist per game