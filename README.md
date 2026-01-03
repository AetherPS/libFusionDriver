# 🌀 libFusionDriver
libFusionDriver is the userland component designed to interface with the Fusion kernel driver. It provides a light weight API, unlocking capabilities for research and homebrew development projects.

## ✨ Key Features and Capabilities
* Exposes low-level functions useful for studying system internals and **Kernel memory**.
* Offers APIs for **reading and writing memory** in any process.
* Allows for **allocating memory** within any process address space.
* Includes an API to inject and **start threads using shellcode** on any process.
* Provides functionality to **start and stop SPRX modules** within any process.

## 🏗️ Build Requirements

### For Official SDK Build
- PS4 Official SDK
- [NASM](https://www.nasm.us/)
- Visual Studio

### For Open Orbis SDK Build
- [Open Orbis SDK](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain)
- [NASM](https://www.nasm.us/)

## 🛠️ Build Steps

1.  Ensure that **NASM** is included in your system's `PATH` environment variable.
2.  **Clone the Repository** and its submodules:
    ```bash
    git clone --recurse-submodules https://github.com/AetherPS/libFusionDriver
    ```
3.  Choose your SDK and build:
    - **Official SDK**: Ensure the PS4 Official SDK is fully installed, including Visual Studio integration. Build the `libFusionDriver` project using **Visual Studio**.
    - **Open Orbis SDK**: Ensure the Open Orbis SDK is properly installed. Build the `libFusionDriverOOSDK` project using **Visual Studio** or your preferred build system.

# 📚 Consumption Instructions
To consume the **libFusionDriver statically linked library**, you will need to add the following **paths** to your project's **Additional Include Directories**:

* `libFusionDriver\Include`
* `libFusionDriver\External\FusionShared`

Link against the appropriate build output based on your SDK:
* **Official SDK**: Link against the library`libFusionDriver.a`
* **Open Orbis SDK**: Link against the library `libFusionDriverOOSDK.a`

# 📘 API Reference

For details on how to use the library see the [API Reference](ApiReference.md).
