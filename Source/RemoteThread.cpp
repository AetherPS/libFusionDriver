#include "stdafx.h"
#include "RemoteThread.h"
#include "FusionDriver.h"
#include "Embed.h"

namespace Fusion
{
	int StartPThread(int processId, uint64_t entryPoint)
	{
		auto shellCodeHeader = (ThreadShellCodeHeader*)&_binary_ThreadShellCode_bin_start;
		shellCodeHeader->ShellCodeComplete = 0;
		shellCodeHeader->ThreadEntry = entryPoint; // Set the thread entry point.

		int res = Resolve(processId, 8193, "libkernel", "scePthreadCreate", 0, &shellCodeHeader->scePthreadCreate);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadCreate.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, 8193, "libkernel", "scePthreadJoin", 0, &shellCodeHeader->scePthreadJoin);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadJoin.\n", __FUNCTION__);
			return -1;
		}

		res = Resolve(processId, 8193, "libkernel", "scePthreadExit", 0, &shellCodeHeader->scePthreadExit);
		if (res != 0)
		{
			klog("%s: Failed to resolve scePthreadExit.\n", __FUNCTION__);
			return -1;
		}

		uint64_t environ = 0;
		res = Resolve(processId, 8193, 0, "environ", 0, &environ);
		if (res != 0)
		{
			klog("%s: Failed to resolve 'environ'... [%llX] Can not continue.\n", __FUNCTION__, res);
			return -1;
		}

		// thr_initial is always +0x10 from the environ symbol.
		shellCodeHeader->thr_initial = environ + 0x10;

		auto shellcodeSize = (uint64_t)&_binary_ThreadShellCode_bin_end - (uint64_t)&_binary_ThreadShellCode_bin_start;
		auto totalAllocatedSize = shellcodeSize + STACK_SIZE;
		uint64_t shellCodeMemory;

#ifdef __ORBIS__
		int result = AllocateMemory(processId, &shellCodeMemory, totalAllocatedSize, PROT_READ | PROT_WRITE | PROT_EXEC);
#else
		int result = AllocateMemory(processId, &shellCodeMemory, totalAllocatedSize, VM_PROT_ALL);
#endif

		if (result != 0 || shellCodeMemory <= 0)
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
		bool shellCodeComplete = 0;
		int timeout = 20; // 2 seconds
		while (!shellCodeComplete && timeout > 0)
		{
			sceKernelUsleep(1000 * 500);

			if (ReadWriteMemory(processId, shellCodeMemory + offsetof(ThreadShellCodeHeader, ShellCodeComplete), (void*)&shellCodeComplete, sizeof(shellCodeComplete), false) != 0)
			{
				klog("%s: Failed to read shellCodeComplete.\n", __FUNCTION__);
				FreeMemory(processId, shellCodeMemory, totalAllocatedSize);
				return -1;
			}

			timeout--;
		}

		if (timeout == 0)
		{
			klog("%s: Timeout waiting for shellcode completion\n", __FUNCTION__);
		}
		else
		{
			sceKernelUsleep(1000 * 200);
		}

		FreeMemory(processId, shellCodeMemory, totalAllocatedSize);

		return 0;
	}
}