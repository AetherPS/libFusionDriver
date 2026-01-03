#include "stdafx.h"
#include "RemoteCallerManager.h"

namespace Fusion
{
    std::unique_ptr<RemoteCaller> RemoteCallerManager::s_instance = nullptr;
    std::mutex RemoteCallerManager::s_mutex;

    RemoteCaller* RemoteCallerManager::GetInstance(int processId)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (!s_instance || s_instance->GetProcessId() != processId)
        {
            s_instance = std::make_unique<RemoteCaller>(processId);
        }

        return s_instance.get();
    }

    RemoteCaller* RemoteCallerManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_instance.get();
    }

    void RemoteCallerManager::Destroy()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_instance.reset();
    }

    bool RemoteCallerManager::HasInstance(int processId)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_instance && s_instance->GetProcessId() == processId;
    }
}