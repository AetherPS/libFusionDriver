# 🌀 libFusionDriver
libFusionDriver is the userland component designed to interface with the Fusion kernel driver. It provides a light weight API, unlocking capabilities for research and homebrew development projects.

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

For details on how to use the library see the [API Reference](ApiReference.md).
