#include "stdafx.h"
#include "RemoteCaller.h"

namespace Fusion
{
    // Reset state for a new call
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
        if (m_header->scratchOffset + size > FUNC_CALL_SCRATCH_SIZE)
            return 0;

        // Copy data to local scratch
        memcpy(&m_header->scratchMemory[m_header->scratchOffset], data, size);

        // Calculate what the remote address will be
        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->scratchOffset += size;

        // Align to 8 bytes for next allocation
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return remotePtr;
    }

    uint64_t RemoteCaller::AllocateScratch(size_t size)
    {
        if (m_header->scratchOffset + size > FUNC_CALL_SCRATCH_SIZE)
            return 0;

        // Zero the memory
        memset(&m_header->scratchMemory[m_header->scratchOffset], 0, size);

        // Calculate remote address
        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->scratchOffset += size;
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return remotePtr;
    }

    bool RemoteCaller::Allocate()
    {
        if (m_allocated)
            return true;

        int res = AllocateMemory(m_processId, &m_remoteAddress, m_totalSize,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_ANON | MAP_PREFAULT_READ);
        if (res != 0 || m_remoteAddress == 0)
        {
            klog("FuncCallInstance: Failed to allocate remote memory\n");
            return false;
        }

        m_allocated = true;
        return true;
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

    bool RemoteCaller::PushString(const char* str, size_t len)
    {
        if (m_header->argCount >= FUNC_CALL_MAX_ARGS)
            return false;

        if (m_header->scratchOffset + len > FUNC_CALL_SCRATCH_SIZE)
            return false;

        // Copy string to scratch
        memcpy(&m_header->scratchMemory[m_header->scratchOffset], str, len);

        // The actual pointer will be calculated when we know the remote address
        // For now, store the offset and we'll fix it up before execution
        // Actually, we need to pre-allocate to know the address...

        // Ensure we have remote memory allocated first
        if (!m_allocated)
        {
            if (!Allocate())
                return false;
        }

        // Calculate remote pointer
        uint64_t remotePtr = m_remoteAddress +
            offsetof(FuncCallHeader, scratchMemory) +
            m_header->scratchOffset;

        m_header->argBuffer[m_header->argCount++] = remotePtr;

        m_header->scratchOffset += len;
        m_header->scratchOffset = (m_header->scratchOffset + 7) & ~7ULL;

        return true;
    }
}