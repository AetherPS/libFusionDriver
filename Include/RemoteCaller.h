#pragma once
#include "FusionDriver.h"
#include "RemoteThread.h"

#define FUNC_CALL_MAGIC 0x4C4C4143  // 'CALL' in little endian
#define FUNC_CALL_MAX_ARGS 16
#define FUNC_CALL_SCRATCH_SIZE 4096

namespace Fusion
{
    // Must match the shellcode layout exactly
    struct FuncCallHeader
    {
        uint32_t magic;                             // 0x00: Magic 'CALL'
        uint64_t entry;                             // 0x04: Shellcode entry point
        uint64_t funcPtr;                           // 0x0C: Target function to call
        uint64_t returnValue;                       // 0x14: Return value after call
        uint64_t argCount;                          // 0x1C: Number of arguments
        uint64_t argBuffer[FUNC_CALL_MAX_ARGS];     // 0x24: Arguments array
        uint64_t scratchOffset;                     // 0xA4: Current scratch memory offset
        uint8_t scratchMemory[FUNC_CALL_SCRATCH_SIZE]; // 0xAC: Scratch memory for strings/data
    } __attribute__((packed));

    class RemoteCaller
    {
    private:
        int m_processId;
        FuncCallHeader* m_header;
        uint64_t m_remoteAddress;
        uint64_t m_totalSize;
        bool m_allocated;
        std::mutex m_mutex;

    public:
        explicit RemoteCaller(int processId);
        ~RemoteCaller();

        // Non-copyable
        RemoteCaller(const RemoteCaller&) = delete;
        RemoteCaller& operator=(const RemoteCaller&) = delete;

        // Get process ID
        int GetProcessId() const;

        // Change target process (will reallocate on next call)
        void SetProcessId(int processId);

        // Reset state for a new call
        void Reset();

        // Set the function to call
        void SetFunction(uint64_t funcAddr);

        // Push an integral argument
        template<typename T>
        typename std::enable_if<std::is_integral<T>::value, bool>::type
            PushArg(T value)
        {
            if (m_header->argCount >= FUNC_CALL_MAX_ARGS)
                return false;

            m_header->argBuffer[m_header->argCount++] = static_cast<uint64_t>(value);
            return true;
        }

        // Push a pointer argument
        template<typename T>
        typename std::enable_if<std::is_pointer<T>::value, bool>::type
            PushArg(T value)
        {
            if (m_header->argCount >= FUNC_CALL_MAX_ARGS)
                return false;

            m_header->argBuffer[m_header->argCount++] = reinterpret_cast<uint64_t>(value);
            return true;
        }

        // Push a string argument - copies to scratch memory and pushes pointer
        bool PushArg(const std::string& str);

        // Push a C-string argument
        bool PushArg(const char* str);

        // Push raw data to scratch and get back the remote pointer
        uint64_t PushData(const void* data, size_t size);

        // Allocate space in scratch memory (returns remote pointer, fills with zeros)
        uint64_t AllocateScratch(size_t size);

        // Get remaining scratch space
        size_t GetScratchRemaining() const;

        // Execute the function call (thread-safe)
        template<typename TReturn = int64_t>
        TReturn Execute()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return ExecuteInternal<TReturn>();
        }

        // Execute with inline arguments (variadic template, thread-safe)
        template<typename TReturn = int64_t, typename... Args>
        TReturn Call(uint64_t funcAddr, Args&&... args)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            Reset();
            SetFunction(funcAddr);
            (PushArg(std::forward<Args>(args)), ...);
            return ExecuteInternal<TReturn>();
        }

        // Read data back from scratch memory after execution
        bool ReadScratch(uint64_t remotePtr, void* outData, size_t size);

        // Cleanup remote memory
        void Cleanup();

        // Get remote address (useful for debugging)
        uint64_t GetRemoteAddress() const;

    private:
        bool EnsureAllocated();

        template<typename TReturn>
        TReturn ExecuteInternal()
        {
            if (!EnsureAllocated())
                return static_cast<TReturn>(-1);

            if (!WriteShellcode())
                return static_cast<TReturn>(-1);

            if (!ExecuteThread())
                return static_cast<TReturn>(-1);

            uint64_t returnValue = 0;
            if (!ReadReturnValue(&returnValue))
                return static_cast<TReturn>(-1);

            return static_cast<TReturn>(returnValue);
        }

        bool PushString(const char* str, size_t len);
        bool WriteShellcode();
        bool ExecuteThread();
        bool ReadReturnValue(uint64_t* outValue);
    };
}
