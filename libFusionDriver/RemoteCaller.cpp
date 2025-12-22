#include "stdafx.h"
#include "RemoteCaller.h"

namespace Fusion
{
    RemoteCaller::RemoteCaller(int processId)
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

    RemoteCaller::~RemoteCaller()
    {
        Cleanup();
    }

    int RemoteCaller::GetProcessId() const
    {
        return m_processId;
    }

    void RemoteCaller::SetProcessId(int processId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_processId != processId)
        {
            Cleanup();
            m_processId = processId;
        }
    }

    void RemoteCaller::Reset()
    {
        m_header->funcPtr = 0;
        m_header->returnValue = 0;
        m_header->argCount = 0;
        m_header->scratchOffset = 0;
        memset(m_header->argBuffer, 0, sizeof(m_header->argBuffer));
    }

    void RemoteCaller::SetFunction(uint64_t funcAddr)
    {
        m_header->funcPtr = funcAddr;
    }

    bool RemoteCaller::PushArg(const std::string& str)
    {
        return PushString(str.c_str(), str.length() + 1);
    }

    bool RemoteCaller::PushArg(const char* str)
    {
        if (str == nullptr)
        {
            return PushArg(static_cast<uint64_t>(0));
        }
        return PushString(str, strlen(str) + 1);
    }

    uint64_t RemoteCaller::PushData(const void* data, size_t size)
    {
        if (!EnsureAllocated())
            return 0;

        if (m_header->scratchOffset + size > FUNC_CALL_SCRATCH_SIZE)
            return 0;

        memcpy(&m_header->scratchMemory[m_header->scratchOffset], data, size);

        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->scratchOffset += size;
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return remotePtr;
    }

    uint64_t RemoteCaller::AllocateScratch(size_t size)
    {
        if (!EnsureAllocated())
            return 0;

        if (m_header->scratchOffset + size > FUNC_CALL_SCRATCH_SIZE)
            return 0;

        memset(&m_header->scratchMemory[m_header->scratchOffset], 0, size);

        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->scratchOffset += size;
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return remotePtr;
    }

    size_t RemoteCaller::GetScratchRemaining() const
    {
        return FUNC_CALL_SCRATCH_SIZE - m_header->scratchOffset;
    }

    bool RemoteCaller::ReadScratch(uint64_t remotePtr, void* outData, size_t size)
    {
        return ReadWriteMemory(m_processId, remotePtr, outData, size, false) == 0;
    }

    void RemoteCaller::Cleanup()
    {
        if (m_allocated && m_remoteAddress != 0)
        {
            FreeMemory(m_processId, m_remoteAddress, m_totalSize);
            m_remoteAddress = 0;
            m_allocated = false;
        }
    }

    uint64_t RemoteCaller::GetRemoteAddress() const
    {
        return m_remoteAddress;
    }

    bool RemoteCaller::EnsureAllocated()
    {
        if (m_allocated)
            return true;

        int res = AllocateMemory(m_processId, &m_remoteAddress, m_totalSize,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_ANON | MAP_PREFAULT_READ);
        if (res != 0 || m_remoteAddress == 0)
        {
            klog("RemoteCaller: Failed to allocate remote memory\n");
            return false;
        }

        m_allocated = true;
        return true;
    }

    bool RemoteCaller::PushString(const char* str, size_t len)
    {
        if (m_header->argCount >= FUNC_CALL_MAX_ARGS)
            return false;

        if (m_header->scratchOffset + len > FUNC_CALL_SCRATCH_SIZE)
            return false;

        if (!EnsureAllocated())
            return false;

        memcpy(&m_header->scratchMemory[m_header->scratchOffset], str, len);

        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->argBuffer[m_header->argCount++] = remotePtr;

        m_header->scratchOffset += len;
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return true;
    }

    bool RemoteCaller::WriteShellcode()
    {
        if (ReadWriteMemory(m_processId, m_remoteAddress, m_header, m_totalSize, true) != 0)
        {
            klog("RemoteCaller: Failed to write shellcode\n");
            return false;
        }
        return true;
    }

    bool RemoteCaller::ExecuteThread()
    {
        uint64_t entryPoint = m_remoteAddress + m_header->entry;
        if (StartPThread(m_processId, entryPoint) != 0)
        {
            klog("RemoteCaller: Thread execution failed\n");
            return false;
        }
        return true;
    }

    bool RemoteCaller::ReadReturnValue(uint64_t* outValue)
    {
        if (ReadWriteMemory(m_processId, m_remoteAddress + offsetof(FuncCallHeader, returnValue),
            outValue, sizeof(uint64_t), false) != 0)
        {
            klog("RemoteCaller: Failed to read return value\n");
            return false;
        }
        return true;
    }
}