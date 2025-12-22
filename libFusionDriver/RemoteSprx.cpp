#include "stdafx.h"
#include "RemoteSprx.h"

namespace Fusion
{
    int LoadSprx(int processId, const std::string& path, int* outResult)
    {
        auto caller = RemoteCallerManager::GetInstance(processId);
        if (!caller)
            return -1;

        int libkernelHandle = Fusion::GetLibraryHandle(processId, "libkernel");
        if (libkernelHandle < 0)
            return -1;

        uint64_t sceKernelLoadStartModule = 0;
        if (Fusion::Resolve(processId, libkernelHandle, "libkernel",
            "sceKernelLoadStartModule", 0, &sceKernelLoadStartModule) != 0)
            return -1;

        // Allocate scratch for output result
        uint64_t remoteResultPtr = caller->AllocateScratch(sizeof(int));
        if (remoteResultPtr == 0)
            return -1;

        int res = caller->Call<int>(sceKernelLoadStartModule,
            path,           // arg1: path
            0ULL,           // arg2: args
            0ULL,           // arg3: argp,
            0ULL,           // arg4: flags  
            0ULL,           // arg5: pOpt
            remoteResultPtr // arg6: pRes
        );

        // Read back the result if requested
        if (outResult != nullptr)
        {
            caller->ReadScratch(remoteResultPtr, outResult, sizeof(int));
        }

        return res;
    }

    int UnloadSprx(int processId, int moduleHandle, int* outResult)
    {
        auto caller = RemoteCallerManager::GetInstance(processId);
        if (!caller)
            return -1;

        int libkernelHandle = Fusion::GetLibraryHandle(processId, "libkernel");
        if (libkernelHandle < 0)
            return -1;

        uint64_t sceKernelStopUnloadModule = 0;
        if (Fusion::Resolve(processId, libkernelHandle, "libkernel",
            "sceKernelStopUnloadModule", 0, &sceKernelStopUnloadModule) != 0)
            return -1;

        // Allocate scratch for output result
        uint64_t remoteResultPtr = caller->AllocateScratch(sizeof(int));
        if (remoteResultPtr == 0)
            return -1;

        int res = caller->Call<int>(sceKernelStopUnloadModule,
            moduleHandle,       // arg1: module handle
            0ULL,               // arg2: args
            0ULL,               // arg3: argp
            0U,                 // arg4: flags
            0ULL,               // arg5: pOpt
            remoteResultPtr     // arg6: pRes
        );

        // Read back the result if requested
        if (outResult != nullptr)
        {
            caller->ReadScratch(remoteResultPtr, outResult, sizeof(int));
        }

        return res;
    }

}