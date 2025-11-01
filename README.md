# 🌀 libFusionDriver: Userland API for the Fusion Kernel Driver
libFusionDriver is the essential userland component designed to interface with a powerful custom kernel driver. It provides a robust and flexible API, unlocking capabilities for advanced research and homebrew development projects.

## ✨ Key Features and Capabilities
* Exposes low-level functions useful for studying system internals and **Kernel memory**.
* Offers APIs for **reading and writing memory** in any process.
* Allows for **allocating memory** within any process address space.
* Includes an API to inject and **start threads using shellcode** on any process.
* Provides functionality to **start and stop SPRX modules** within any process.

## 🏗️ Build Requirements
- PS4 Official SDK
- [NASM](https://www.nasm.us/)

## 🛠️ Build Steps

1.  Ensure that **NASM** is included in your system's `PATH` environment variable.
2.  Ensure that the **PS4 Official SDK** is fully installed, including the Visual Studio integration components.
3.  **Clone the Repository** and its submodules:
    ```bash
    git clone --recurse-submodules https://github.com/AetherPS/libFusionDriver
    ```
4.  Build the project `libFusionDriver` using **Visual Studio**.

# 📚 Consumtion Instructions
To consume the **libFusionDriver statically linked library**, you will need to add the following **paths** to your project's **Additional Include Directories**:

* `libFusionDriver\libFusionDriver`
* `libFusionDriver\External\FusionShared`

# 📘 API Reference

### FusionDriver.h
``` C++
bool IsDriverLoaded();
int GetDriverInfo(FusionDriverInfo* info);
int Jailbreak(int processId, JailBackup* backup, uint64_t authId = -1, bool nullSandboxPath = false);
int RestoreJail(int processId, JailBackup backup);
int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount, int* realCount);
int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write);
int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags);
int FreeMemory(int processId, uint64_t processAddress, size_t length);
int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize);
int Resolve(int processId, int libHandle, const char* library, const char* symbol, uint64_t* addr);
int GetAuthId(int processId, uint64_t* authId);
int SetAuthId(int processId, uint64_t authId);
```

### ShellCode.h
``` C++
int StartPThread(int processId, uint64_t entryPoint);
int LoadSprx(int processId, const char* path);
int UnloadSprx(int processId, int handle);
```