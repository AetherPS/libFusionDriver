#include "stdafx.h"
#include "RemoteThread.h"
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

		auto shellCodeHeader = (ThreadShellCodeHeader*)&_binary_ThreadShellCode_bin_start;
		shellCodeHeader->ShellCodeComplete = 0;
		shellCodeHeader->ThreadEntry = entryPoint; // Set the thread entry point.

		int res = Resolve(processId, libkernelHandle, "libkernel", "scePthreadCreate", 0, &shellCodeHeader->scePthreadCreate);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadCreate.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, libkernelHandle, "libkernel", "scePthreadJoin", 0, &shellCodeHeader->scePthreadJoin);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadJoin.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, libkernelHandle, "libkernel", "scePthreadExit", 0, &shellCodeHeader->scePthreadExit);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadExit.\n", __FUNCTION__);
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

		auto shellcodeSize = (uint64_t)&_binary_ThreadShellCode_bin_end - (uint64_t)&_binary_ThreadShellCode_bin_start;
		auto totalAllocatedSize = shellcodeSize + STACK_SIZE;
		uint64_t shellCodeMemory;

#ifdef __ORBIS__
		int result = AllocateMemory(processId, &shellCodeMemory, totalAllocatedSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PREFAULT_READ);
#else
		int result = AllocateMemory(processId, &shellCodeMemory, totalAllocatedSize, VM_PROT_ALL, MAP_ANON | MAP_PREFAULT_READ);
#endif

		if (result != 0 || (void*)shellCodeMemory == nullptr || (void*)shellCodeMemory == MAP_FAILED || shellCodeMemory < 0)
		{
			klog("%s: Failed to allocate memory on the process. %llX\n", __FUNCTION__, shellCodeMemory);
			return -1;
		}

		if (ReadWriteMemory(processId, shellCodeMemory, shellCodeHeader, shellcodeSize, true) != 0)
		{
			klog("%s: Write shellcode failed.\n", __FUNCTION__);
			FreeMemory(processId, shellCodeMemory, totalAllocatedSize);
			return false;
		}

		// Create a thread to run our shellcode.
		StartThread(processId, shellCodeMemory + shellCodeHeader->entry, shellCodeMemory + shellcodeSize, STACK_SIZE);

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
}