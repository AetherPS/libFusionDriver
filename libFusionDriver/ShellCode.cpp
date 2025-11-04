#include "stdafx.h"
#include "ShellCode.h"
#include "FusionDriver.h"
#include "Embed.h"

namespace Fusion
{
	int StartPThread(int processId, uint64_t entryPoint)
	{
		int libkernelHandle = GetLibraryHandle(processId, "libkernel");
		if (libkernelHandle < 0)
		{
			klog("%s: Failed to get libkernel handle.\n", __FUNCTION__);
			return -1;
		}

		auto shellCodeHeader = (ThreadShellCodeHeader*)_binary_ThreadShellCode_bin_start;
		shellCodeHeader->ShellCodeComplete = 0;
		shellCodeHeader->ThreadEntry = entryPoint; // Set the thread entry point.

		int res = Resolve(processId, libkernelHandle, nullptr, "scePthreadCreate", &shellCodeHeader->scePthreadCreate);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadCreate.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, libkernelHandle, nullptr, "scePthreadJoin", &shellCodeHeader->scePthreadJoin);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadJoin.\n", __FUNCTION__);
			return -1;
		}

		// Find the thread initial.
		shellCodeHeader->thr_initial = GetRemoteAddress(processId, "libkernel.sprx", 0x0008E430);
		
		if (shellCodeHeader->thr_initial == 0)
			shellCodeHeader->thr_initial = GetRemoteAddress(processId, "libkernel_sys.sprx", 0x0008E830);
		
		if (shellCodeHeader->thr_initial == 0)
			shellCodeHeader->thr_initial = GetRemoteAddress(processId, "libkernel_web.sprx", 0x0008E430);

		// Make sure that we actually found thr_initial.
		if (shellCodeHeader->thr_initial <= 0)
		{
			klog("%s: Failed to resolve thr_initial.\n", __FUNCTION__);
			return -1;
		}

		auto totalAllocatedSize = _binary_ThreadShellCode_bin_size + STACK_SIZE;
		uint64_t shellCodeMemory;
		if (AllocateMemory(processId, &shellCodeMemory, totalAllocatedSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PREFAULT_READ) != 0 || (void*)shellCodeMemory == nullptr || (void*)shellCodeMemory == MAP_FAILED || shellCodeMemory < 0)
		{
			klog("%s: Failed to allocate memory on the process. %llX\n", __FUNCTION__, shellCodeMemory);
			return -1;
		}

		if (ReadWriteMemory(processId, shellCodeMemory, shellCodeHeader, _binary_ThreadShellCode_bin_size, true) != 0)
		{
			klog("%s: Write shellcode failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, totalAllocatedSize);
			return false;
		}

		// Create a thread to run our shellcode.
		StartThread(processId, shellCodeMemory + shellCodeHeader->entry, shellCodeMemory + _binary_ThreadShellCode_bin_size, STACK_SIZE);

		// Wait for the shellcode to complete by reading the byte that will be set to 1 on completion.
		bool shellCodeComplete = false;
		while (!shellCodeComplete)
		{
			sceKernelUsleep(1000 * 500);

			if (ReadWriteMemory(processId, shellCodeMemory + offsetof(ThreadShellCodeHeader, ShellCodeComplete), (void*)&shellCodeComplete, sizeof(shellCodeComplete), false) != 0)
			{
				klog("%s: Failed to read shellCodeComplete.\n", __FUNCTION__);
				FreeMemory(processId, shellCodeMemory, totalAllocatedSize);
				return -1;
			}
		}

		FreeMemory(processId, shellCodeMemory, totalAllocatedSize);

		return 0;
	}

	int LoadSprx(int processId, const char* path)
	{
		int libkernelHandle = GetLibraryHandle(processId, "libkernel");
		if (libkernelHandle < 0)
		{
			klog("%s: Failed to get libkernel handle.\n", __FUNCTION__);
			return -1;
		}

		auto shellCodeHeader = (SprxLoaderHeader*)_binary_LoaderShellCode_bin_start;
		shellCodeHeader->ModuleHandle = -1;
		strcpy(shellCodeHeader->Path, (char*)path);

		int res = Resolve(processId, libkernelHandle, nullptr, "sceKernelLoadStartModule", &shellCodeHeader->sceKernelLoadStartModule);
		if (res != 0)
		{
			klog("%s: Failed to resolve sceKernelLoadStartModule.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, libkernelHandle, nullptr, "scePthreadExit", &shellCodeHeader->scePthreadExit);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadExit.\n", __FUNCTION__);
			return -1;
		}

		uint64_t shellCodeMemory;
		if (AllocateMemory(processId, &shellCodeMemory, _binary_LoaderShellCode_bin_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PREFAULT_READ) != 0 || (void*)shellCodeMemory == nullptr || (void*)shellCodeMemory == MAP_FAILED || shellCodeMemory < 0)
		{
			klog("%s: Failed to allocate memory on the process. %llX\n", __FUNCTION__, shellCodeMemory);
			return -1;
		}

		if (ReadWriteMemory(processId, shellCodeMemory, shellCodeHeader, _binary_LoaderShellCode_bin_size, true) != 0)
		{
			klog("%s: Write shellcode failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_LoaderShellCode_bin_size);
			return false;
		}

		if (StartPThread(processId, (uint64_t)(shellCodeMemory + shellCodeHeader->entry)) != 0)
		{
			klog("%s: shellcode thread failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_LoaderShellCode_bin_size);
			return false;
		}
		
		// Read out the module handle to see if the loading was a success or not.
		int32_t moduleHandle = -1;
		if (ReadWriteMemory(processId, shellCodeMemory + offsetof(SprxLoaderHeader, ModuleHandle), (void*)&moduleHandle, sizeof(moduleHandle), false) != 0)
		{
			klog("%s: Failed to read ModuleHandle.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_LoaderShellCode_bin_size);
			return -1;
		}
		
		// Free up the memory we dont need it anymore.
		FreeMemory(processId, shellCodeMemory, _binary_LoaderShellCode_bin_size);
		
		return moduleHandle;
	}

	int UnloadSprx(int processId, int handle)
	{
		int libkernelHandle = GetLibraryHandle(processId, "libkernel");
		if (libkernelHandle < 0)
		{
			klog("%s: Failed to get libkernel handle.\n", __FUNCTION__);
			return -1;
		}

		auto shellCodeHeader = (SprxUnLoaderHeader*)_binary_UnLoaderShellCode_bin_start;
		shellCodeHeader->ModuleHandle = handle;
		shellCodeHeader->Result = 0;

		int res = Resolve(processId, libkernelHandle, nullptr, "sceKernelStopUnloadModule", &shellCodeHeader->sceKernelStopUnloadModule);
		if (res != 0)
		{
			klog("%s: Failed to resolve sceKernelStopUnloadModule.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, libkernelHandle, nullptr, "scePthreadExit", &shellCodeHeader->scePthreadExit);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadExit.\n", __FUNCTION__);
			return -1;
		}

		uint64_t shellCodeMemory;
		if (AllocateMemory(processId, &shellCodeMemory, _binary_UnLoaderShellCode_bin_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PREFAULT_READ) != 0 || (void*)shellCodeMemory == nullptr || (void*)shellCodeMemory == MAP_FAILED || shellCodeMemory < 0)
		{
			klog("%s: Failed to allocate memory on the process. %llX\n", __FUNCTION__, shellCodeMemory);
			return -1;
		}

		if (ReadWriteMemory(processId, shellCodeMemory, shellCodeHeader, _binary_UnLoaderShellCode_bin_size, true) != 0)
		{
			klog("%s: Write shellcode failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_UnLoaderShellCode_bin_size);
			return -1;
		}

		if (StartPThread(processId, (uint64_t)(shellCodeMemory + shellCodeHeader->entry)) != 0)
		{
			klog("%s: shellcode thread failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_UnLoaderShellCode_bin_size);
			return -1;
		}

		// Read the result from the shellcode to see if we succeeded.
		int32_t result = -1;
		if (ReadWriteMemory(processId, shellCodeMemory + offsetof(SprxUnLoaderHeader, Result), (void*)&result, sizeof(result), false) != 0)
		{
			klog("%s: Failed to read Result.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, _binary_UnLoaderShellCode_bin_size);
			return -1;
		}

		FreeMemory(processId, shellCodeMemory, _binary_UnLoaderShellCode_bin_size);

		return result;
	}
}