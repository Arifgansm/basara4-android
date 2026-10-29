# FEX NDK Atomic Ref Patch

This patch adds a compatibility shim for std::atomic_ref which is not available
in NDK's libc++. It provides a lightweight wrapper that mimics atomic_ref
semantics using std::atomic operations.

## Files Modified

1. `FEXCore/include/FEXCore/Utils/SpinWaitLock.h` - Replace `std::atomic_ref` usage
2. `FEXCore/include/FEXCore/Utils/WritePriorityMutex.h` - Replace `std::atomic_ref` usage  
3. `FEXCore/include/FEXCore/Utils/SHMStats.h` - Replace `std::atomic_ref` usage

## Approach

Create an `fex_atomic_ref` wrapper class that:
- Wraps a non-atomic type
- Provides the same load/store/compare_exchange_strong/fetch_add API as std::atomic_ref
- Uses compiler intrinsics and proper memory ordering for atomic operations
- Only enabled when building for Android/NDK (where std::atomic_ref is missing)

## Installation

Apply this patch after checking out FEX submodule:

```bash
git -C third_party/FEX apply ../fex-ndk-atomic-ref.patch
```

## Notes

The shim is designed to be a drop-in replacement with the same API surface.
For production code, consider upstreaming these changes to FEX or using
a newer NDK that includes full C++20 support.