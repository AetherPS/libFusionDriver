#pragma once
#include "Embed.h"
#include "RemoteThread.h"
#include "RemoteCaller.h"

namespace Fusion
{
    class RemoteCallerManager
    {
    private:
        static std::unique_ptr<RemoteCaller> s_instance;
        static std::mutex s_mutex;

    public:
        // Get or create the global instance for a process
        static RemoteCaller* GetInstance(int processId);

        // Get existing instance (returns nullptr if none exists)
        static RemoteCaller* GetInstance();

        // Destroy the global instance
        static void Destroy();

        // Check if instance exists for a specific process
        static bool HasInstance(int processId);
    };
}
