# 📘 API Reference

## FusionDriver.h

Core driver interface for process manipulation and memory operations.

### Driver Status

```cpp
bool IsDriverLoaded();
```
Returns `true` if the Fusion kernel driver is currently loaded and operational.

---

```cpp
int GetDriverInfo(FusionDriverInfo* info);
```
Retrieves information about the loaded driver.

| Parameter | Type | Description |
|-----------|------|-------------|
| `info` | `FusionDriverInfo*` | Output structure to receive driver information |

**Returns:** `0` on success, error code on failure.

---

### Process Manipulation

```cpp
int Jailbreak(int processId, JailBackup* backup, uint64_t authId = -1, bool nullSandboxPath = false);
```
Escapes the sandbox for the specified process, optionally elevating privileges.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `backup` | `JailBackup*` | Output structure to store original jail state for restoration |
| `authId` | `uint64_t` | Auth Id to set (default: `-1` to keep current) |
| `nullSandboxPath` | `bool` | If `true`, nullifies the sandbox path |

**Returns:** `0` on success, error code on failure.

---

```cpp
int RestoreJail(int processId, JailBackup backup);
```
Restores a process to its original sandboxed state.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `backup` | `JailBackup` | Backup structure from previous `Jailbreak` call |

**Returns:** `0` on success, error code on failure.

---

```cpp
int GetAuthId(int processId, uint64_t* authId);
```
Retrieves the authentication Id for a process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `authId` | `uint64_t*` | Output pointer to receive the auth Id |

**Returns:** `0` on success, error code on failure.

---

```cpp
int SetAuthId(int processId, uint64_t authId);
```
Sets the authentication Id for a process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `authId` | `uint64_t` | New auth Id to assign |

**Returns:** `0` on success, error code on failure.

---

### Memory Operations

```cpp
int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write);
```
Reads from or writes to a process's memory.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `addr` | `uint64_t` | Remote memory address |
| `data` | `void*` | Local buffer for read/write data |
| `len` | `size_t` | Number of bytes to transfer |
| `write` | `bool` | `true` to write, `false` to read |

**Returns:** `0` on success, error code on failure.

---

```cpp
int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags);
```
Allocates memory in the target process's address space.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `outAddress` | `uint64_t*` | Output pointer to receive allocated address |
| `length` | `size_t` | Size of allocation in bytes |
| `protection` | `int` | Memory protection flags (e.g., `PROT_READ \| PROT_WRITE`) |
| `flags` | `int` | Mapping flags (e.g., `MAP_ANONYMOUS \| MAP_PRIVATE`) |

**Returns:** `0` on success, error code on failure.

---

```cpp
int FreeMemory(int processId, uint64_t processAddress, size_t length);
```
Frees previously allocated memory in the target process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `processAddress` | `uint64_t` | Address to free |
| `length` | `size_t` | Size of the allocation |

**Returns:** `0` on success, error code on failure.

---

### Module & Symbol Resolution

```cpp
int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount, int* realCount);
```
Retrieves a list of loaded libraries/modules in the target process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `libraries` | `OrbisLibraryInfo*` | Output array to receive library info |
| `maxCount` | `int` | Maximum entries the array can hold |
| `realCount` | `int*` | Output pointer to receive actual library count |

**Returns:** `0` on success, error code on failure.

---

```cpp
int Resolve(int processId, int libHandle, const char* library, const char* symbol, unsigned int flags, uint64_t* addr);
```
Resolves a symbol address within a loaded library.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `libHandle` | `int` | Library handle (or `0` to search by name) |
| `library` | `const char*` | Library name (used if `libHandle` is `0`) |
| `symbol` | `const char*` | Symbol name to resolve |
| `flags` | `unsigned int` | Resolution flags |
| `addr` | `uint64_t*` | Output pointer to receive symbol address |

**Returns:** `0` on success, error code on failure.

---

### Thread Execution

```cpp
int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize);
```
Starts a new thread in the target process at a specified entry point.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `threadEntry` | `uint64_t` | Entry point address for the thread |
| `stackMemory` | `uint64_t` | Pre-allocated stack memory address |
| `stackSize` | `size_t` | Size of the stack |

**Returns:** `0` on success, error code on failure.

---

### Kernel Operations

```cpp
int GetKernelBase(uint64_t* kernelBase);
```
Retrieves the base address of the kernel.

| Parameter | Type | Description |
|-----------|------|-------------|
| `kernelBase` | `uint64_t*` | Output pointer to receive kernel base address |

**Returns:** `0` on success, error code on failure.

---

```cpp
int KernelReadWriteMemory(uint64_t addr, void* data, size_t len, bool write);
```
Reads from or writes to kernel memory.

| Parameter | Type | Description |
|-----------|------|-------------|
| `addr` | `uint64_t` | Kernel memory address |
| `data` | `void*` | Local buffer for read/write data |
| `len` | `size_t` | Number of bytes to transfer |
| `write` | `bool` | `true` to write, `false` to read |

**Returns:** `0` on success, error code on failure.

---

```cpp
int KernelReadWriteIccNvs(uint32_t block, uint32_t offset, uint32_t size, uint8_t* value, bool isWrite);
```
Reads from or writes to the ICC NVS (non-volatile storage) in sFlash0.

| Parameter | Type | Description |
|-----------|------|-------------|
| `block` | `uint32_t` | Block index (0: 0x1C4000, 1: 0x1C7000, 2: 0x1C8000, 4: 0x1CC000) |
| `offset` | `uint32_t` | Offset within the block |
| `size` | `uint32_t` | Number of bytes to transfer |
| `value` | `uint8_t*` | Buffer for read/write data |
| `isWrite` | `bool` | `true` to write, `false` to read |

**Returns:** `0` on success, error code on failure.

---

### Helper Functions

```cpp
uint64_t GetRemoteAddress(int processId, const char* library, uint64_t offset);
uint64_t GetRemoteAddress(int processId, int handle, uint64_t offset);
```
Calculates an absolute address from a library base plus offset.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `library` | `const char*` | Library name (first overload) |
| `handle` | `int` | Library handle (second overload) |
| `offset` | `uint64_t` | Offset from library base |

**Returns:** Absolute remote address, or `0` on failure.

---

```cpp
int GetLibraryHandle(int processId, const char* library);
```
Retrieves the handle for a loaded library by name.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `library` | `const char*` | Library name to find |

**Returns:** Library handle on success, `-1` on failure.

---

## RemoteThread.h

Low-level pthread creation in remote processes.

### Functions

```cpp
int StartPThread(int processId, uint64_t entryPoint);
```
Creates and starts a pthread in the target process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `entryPoint` | `uint64_t` | Thread entry point address |

**Returns:** `0` on success, error code on failure.

---

## RemoteSprx.h

SPRX module loading and unloading.

```cpp
int LoadSprx(int processId, const std::string& path, int* outResult = nullptr);
```
Loads an SPRX module into the target process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `path` | `const std::string&` | Path to the SPRX file |
| `outResult` | `int*` | Optional output for the load result |

**Returns:** module handle on success, error code on failure.

---

```cpp
int UnloadSprx(int processId, int moduleHandle, int* outResult = nullptr);
```
Unloads an SPRX module from the target process.

| Parameter | Type | Description |
|-----------|------|-------------|
| `processId` | `int` | Target process Id |
| `moduleHandle` | `int` | Handle of the module to unload |
| `outResult` | `int*` | Optional output for the unload result |

**Returns:** `0` on success, error code on failure.

---

## RemoteCaller.h

High-level remote function call interface with automatic argument marshalling.

### Class: RemoteCaller

A non-copyable class for executing function calls in a remote process.

#### Constructor / Destructor

```cpp
explicit RemoteCaller(int processId);
~RemoteCaller();
```

#### Process Management

```cpp
int GetProcessId() const;
```
Returns the current target process Id.

---

```cpp
void SetProcessId(int processId);
```
Changes the target process. Memory will be reallocated on the next call.

---

#### Call Setup

```cpp
void Reset();
```
Resets the internal state for a new function call.

---

```cpp
void SetFunction(uint64_t funcAddr);
```
Sets the remote function address to call.

---

#### Argument Pushing

```cpp
template<typename T>
bool PushArg(T value);
```
Pushes an integral or pointer argument onto the argument buffer.

**Returns:** `true` if successful, `false` if argument limit reached.

---

```cpp
bool PushArg(const std::string& str);
bool PushArg(const char* str);
```
Pushes a string argument. The string is copied to scratch memory and a remote pointer is pushed.

**Returns:** `true` if successful, `false` if out of scratch space.

---

#### Scratch Memory

```cpp
uint64_t PushData(const void* data, size_t size);
```
Copies raw data to scratch memory.

**Returns:** Remote pointer to the data, or `0` on failure.

---

```cpp
uint64_t AllocateScratch(size_t size);
```
Allocates zero-initialized space in scratch memory.

**Returns:** Remote pointer to the allocation, or `0` on failure.

---

```cpp
size_t GetScratchRemaining() const;
```
Returns the remaining bytes available in scratch memory.

---

```cpp
bool ReadScratch(uint64_t remotePtr, void* outData, size_t size);
```
Reads data back from scratch memory after execution.

**Returns:** `true` on success.

---

#### Execution

```cpp
template<typename TReturn = int64_t>
TReturn Execute();
```
Executes the prepared function call (thread-safe).

**Returns:** The return value cast to `TReturn`, or `-1` on failure.

---

```cpp
template<typename TReturn = int64_t, typename... Args>
TReturn Call(uint64_t funcAddr, Args&&... args);
```
Convenience method to set up and execute a call in one step (thread-safe).

**Returns:** The return value cast to `TReturn`, or `-1` on failure.

**Example:**
```cpp
RemoteCaller caller(pId);

// Simple call.
int result = caller->Call<int>(sceKernelLoadStartModule,
    path,   // arg1: const char* path (auto copied to scratch)
    0ULL,   // arg2: size_t args size
    0ULL,   // arg3: const void* args
    0ULL,   // arg4: uint32_t flags  
    0ULL,   // arg5: const void* option
    0ULL    // arg6: int* result
);

// Or more verbose.
caller.SetFunction(sceKernelLoadStartModule);
caller.PushArg(path);    // arg1: path string
caller.PushArg(0);       // arg2: args size
caller.PushArg(0);       // arg3: args
caller.PushArg(0);       // arg4: flags
caller.PushArg(0);       // arg5: option
caller.PushArg(0);       // arg6: result
int result = call->Execute<int>();
```

---

#### Cleanup

```cpp
void Cleanup();
```
Frees all remote memory allocations.

---

```cpp
uint64_t GetRemoteAddress() const;
```
Returns the remote base address (useful for debugging).

---

## RemoteCallerManager.h

Global singleton manager for `RemoteCaller` instances.

### Class: RemoteCallerManager

```cpp
static RemoteCaller* GetInstance(int processId);
```
Gets or creates the global `RemoteCaller` instance for the specified process. If an instance exists for a different process, it may be replaced.

**Returns:** Pointer to the `RemoteCaller` instance.

---

```cpp
static RemoteCaller* GetInstance();
```
Gets the existing global instance without creating one.

**Returns:** Pointer to the instance, or `nullptr` if none exists.

---

```cpp
static void Destroy();
```
Destroys the global instance and frees all associated resources.

---

```cpp
static bool HasInstance(int processId);
```
Checks if an instance exists for the specified process.

**Returns:** `true` if an instance exists for `processId`.

---

## Error Codes

| Code | Description |
|------|-------------|
| `0` | Success |
| `-1` | Generic failure |
| *Others* | Driver/system specific error codes |

---

## Usage Example

```cpp
#include "FusionDriver.h"
#include "RemoteCaller.h"
#include "RemoteSprx.h"

int main()
{
    if (!Fusion::IsDriverLoaded())
        return -1;

    int pId = /* target process Id */;
    
    // Jailbreak the process
    Fusion::JailBackup backup;
    if (Fusion::Jailbreak(pId, &backup) != 0)
        return -1;

    // Load an SPRX module
    int moduleHandle = Fusion::LoadSprx(pId, "/data/payload.sprx");
    if (moduleHandle != 0)
        return -1;

    // Call a remote function
    auto* caller = Fusion::RemoteCallerManager::GetInstance(pId);
    uint64_t funcAddr = Fusion::GetRemoteAddress(pId, "libSceLibcInternal.sprx", 0x1234);
    int result = caller->Call<int>(funcAddr, 100, "test");

    // Cleanup
    Fusion::RestoreJail(pId, backup);
    Fusion::RemoteCallerManager::Destroy();

    return 0;
}
```