#include "stdafx.h"
#include "FusionDriver.h"

namespace Fusion
{
	bool IsDriverLoaded()
	{
#ifdef __ORBIS__
		SceKernelStat stat;
#else
		OrbisKernelStat stat;
#endif
		return sceKernelStat(DRIVER_PATH, &stat) == 0;
	}

	int MakeDriverRequest(unsigned long request, void* input)
	{
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device.\n");
			return ENOENT;
		}

		int res = ioctl(fd, request, input);
		if (res != 0)
		{
			klog("Driver request 0x%lX failed with %d\n", request, res);
			sceKernelClose(fd);
			return res;
		}

		sceKernelClose(fd);
		return 0;
	}

	int GetDriverInfo(FusionDriverInfo* info)
	{
		return MakeDriverRequest(FUSION_DRIVERINFO, info);
	}

	int Jailbreak(int processId, JailBackup* backup, uint64_t authId, bool nullSandboxPath)
	{
		Input_Jailbreak input;
		input.ProcessId = processId;
		input.Jail = backup;
		input.AuthId = authId;
		input.NullRandPath = nullSandboxPath;

		return MakeDriverRequest(PROC_JAILBREAK, &input);
	}

	int RestoreJail(int processId, JailBackup backup)
	{
		Input_RestoreJail input;
		input.ProcessId = processId;
		input.Jail = backup;

		return MakeDriverRequest(PROC_JAIL, &input);
	}

	int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount, int* realCount)
	{
		Input_LibraryList input;
		input.ProcessId = processId;
		input.LibraryListOut = libraries;
		input.MaxOutCount = maxCount;
		input.LibraryCount = realCount;

		return MakeDriverRequest(PROC_MODULE_LIST, &input);
	}

	int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write)
	{
		Input_ReadWriteMemory input;
		input.ProcessId = processId;
		input.ProcessAddress = addr;
		input.DataAddress = data;
		input.Length = len;
		input.IsWrite = write;

		return MakeDriverRequest(PROC_READ_WRITE_MEMORY, &input);
	}

	int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags)
	{
		Input_AllocMemory input;
		input.ProcessId = processId;
		input.OutAddress = outAddress;
		input.Length = length;
		input.Protection = protection;
		input.Flags = flags;

		return MakeDriverRequest(PROC_ALLOCATE_MEMORY, &input);
	}

	int FreeMemory(int processId, uint64_t processAddress, size_t length)
	{
		Input_FreeMemory input;
		input.ProcessId = processId;
		input.ProcessAddress = processAddress;
		input.Length = length;

		return MakeDriverRequest(PROC_FREE_MEMORY, &input);
	}

	int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize)
	{
		Input_StartThreadInfo input;
		input.ProcessId = processId;
		input.ThreadEntry = threadEntry;
		input.StackMemory = stackMemory;
		input.StackSize = stackSize;

		return MakeDriverRequest(PROC_START_THREAD, &input);
	}

	int Resolve(int processId, int libHandle, const char* library, const char* symbol, unsigned int flags, uint64_t* addr)
	{
		Input_ResolveInfo input;
		input.ProcessId = processId;
		input.Handle = libHandle;
		input.Flags = flags;

		if (library != nullptr)
			strcpy(input.Library, library);

		if (symbol != nullptr)
			strcpy(input.Symbol, symbol);

		int res = MakeDriverRequest(PROC_RESOLVE, &input);

		*addr = input.Address;

		return res;
	}

	int GetAuthId(int processId, uint64_t* authId)
	{
		Input_AuthId input;
		input.ProcessId = processId;
		input.AuthId = 0;

		int res = MakeDriverRequest(PROC_GET_AUTHID, &input);
		if (res != 0)
			return res;

		*authId = input.AuthId;

		return 0;
	}

	int SetAuthId(int processId, uint64_t authId)
	{
		Input_AuthId input;
		input.ProcessId = processId;
		input.AuthId = authId;

		return MakeDriverRequest(PROC_GET_AUTHID, &input);
	}

	int GetKernelBase(uint64_t* kernelBase)
	{
		Input_KernelBase input;
		input.KernelBase = 0;

		int res = MakeDriverRequest(KERN_GET_BASE, &input);
		if (res != 0)
			return res;

		if (kernelBase)
		{
			*kernelBase = input.KernelBase;
		}

		return 0;
	}

	int KernelReadWriteMemory(uint64_t addr, void* data, size_t len, bool write)
	{
		Input_ReadWriteMemory input;
		input.ProcessId = 0;
		input.ProcessAddress = addr;
		input.DataAddress = data;
		input.Length = len;
		input.IsWrite = write;

		return MakeDriverRequest(KERN_READ_WRITE_MEMORY, &input);
	}

	int KernelReadWriteIccNvs(uint32_t block, uint32_t offset, uint32_t size, uint8_t* value, bool isWrite)
	{
		Input_IccNvsReadWrite input;
		input.Block = block;
		input.Offset = offset;
		input.Size = size;
		input.Value = value;
		input.IsWrite = isWrite;

		return MakeDriverRequest(KERN_ICC_NVS_READ_WRITE, &input);
	}

	uint64_t GetRemoteAddress(int processId, const char* library, uint64_t offset)
	{
		OrbisLibraryInfo libraries[255];
		int realCount;
		int res = GetLibraryList(processId, libraries, 255, &realCount);

		if (res != 0)
		{
			klog("%s: GetLibraryList failed with %d\n", __FUNCTION__, res);
			return res;
		}

		for (int i = 0; i < realCount; i++)
		{
			if (strstr(libraries[i].Path, library))
				return libraries[i].MapBase + offset;
		}

		return 0;
	}

	uint64_t GetRemoteAddress(int processId, int handle, uint64_t offset)
	{
		OrbisLibraryInfo libraries[255];
		int realCount;
		int res = GetLibraryList(processId, libraries, 255, &realCount);

		if (res != 0)
		{
			klog("%s: GetLibraryList failed with %d\n", __FUNCTION__, res);
			return res;
		}

		for (int i = 0; i < realCount; i++)
		{
			if (libraries[i].Handle == handle)
				return libraries[i].MapBase + offset;
		}

		return 0;
	}


	int GetLibraryHandle(int processId, const char* library)
	{
		OrbisLibraryInfo libraries[255];
		int realCount;
		int res = GetLibraryList(processId, libraries, 255, &realCount);

		if (res != 0)
		{
			klog("%s: GetLibraryList failed with %d\n", __FUNCTION__, res);
			return res;
		}

		for (int i = 0; i < realCount; i++)
		{
			if (strstr(libraries[i].Path, library))
				return libraries[i].Handle;
		}

		return -1;
	}
}