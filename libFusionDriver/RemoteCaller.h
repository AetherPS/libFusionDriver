#pragma once
#include "FusionDriver.h"
#include "ShellCode.h"

namespace Fusion
{
#define FUNC_CALL_MAGIC 0x4C4C4143  // 'CALL' in little endian
#define FUNC_CALL_MAX_ARGS 16
#define FUNC_CALL_SCRATCH_SIZE 4096

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

    public:
        RemoteCaller(int processId)
            : m_processId(processId)
            , m_header(nullptr)
            , m_remoteAddress(0)
            , m_totalSize(0)
            , m_allocated(false)
        {
            m_totalSize = (uint64_t)&_binary_FuncCallShellCode_bin_end -
                (uint64_t)&_binary_FuncCallShellCode_bin_start;
            m_header = reinterpret_cast<FuncCallHeader*>(&_binary_FuncCallShellCode_bin_start);
            Reset();
        }

        ~RemoteCaller()
        {
            Cleanup();
        }

        void Reset();
        void SetFunction(uint64_t funcAddr);
        bool PushArg(const std::string& str);
        bool PushArg(const char* str);
        uint64_t PushData(const void* data, size_t size);
        uint64_t AllocateScratch(size_t size);
        bool Allocate();
        bool ReadScratch(uint64_t remotePtr, void* outData, size_t size);
        void Cleanup();

        // Push a numeric argument (integers, pointers as uint64_t)
        template<typename T>
        typename std::enable_if<std::is_integral<T>::value || std::is_pointer<T>::value, bool>::type
            PushArg(T value)
        {
            if (m_header->argCount >= FUNC_CALL_MAX_ARGS)
                return false;

            m_header->argBuffer[m_header->argCount++] = static_cast<uint64_t>(
                reinterpret_cast<uintptr_t>(value)
                );
            return true;
        }

        size_t GetScratchRemaining() const
        {
            return FUNC_CALL_SCRATCH_SIZE - m_header->scratchOffset;
        }

        template<typename TReturn = int64_t>
        TReturn Execute()
        {
            if (!m_allocated)
            {
                if (!Allocate())
                    return static_cast<TReturn>(-1);
            }

            // Write shellcode + header to remote process
            if (ReadWriteMemory(m_processId, m_remoteAddress, m_header, m_totalSize, true) != 0)
            {
                klog("FuncCallInstance: Failed to write shellcode\n");
                return static_cast<TReturn>(-1);
            }

            // Execute
            uint64_t entryPoint = m_remoteAddress + m_header->entry;
            if (StartPThread(m_processId, entryPoint) != 0)
            {
                klog("FuncCallInstance: Thread execution failed\n");
                return static_cast<TReturn>(-1);
            }

            // Read back return value
            uint64_t returnValue = 0;
            if (ReadWriteMemory(m_processId, m_remoteAddress + offsetof(FuncCallHeader, returnValue),
                &returnValue, sizeof(returnValue), false) != 0)
            {
                klog("FuncCallInstance: Failed to read return value\n");
                return static_cast<TReturn>(-1);
            }

            return static_cast<TReturn>(returnValue);
        }

        // Execute with inline arguments (variadic template)
        template<typename TReturn = int64_t, typename... Args>
        TReturn Call(uint64_t funcAddr, Args&&... args)
        {
            Reset();
            SetFunction(funcAddr);
            (PushArg(std::forward<Args>(args)), ...);
            return Execute<TReturn>();
        }

        // Get remote address (useful for debugging)
        uint64_t GetRemoteAddress() const { return m_remoteAddress; }

    private:
        bool PushString(const char* str, size_t len);
    };
}
