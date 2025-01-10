#include "stdafx.h"
#include "FusionDriver.h"
#include "ShellCode.h"

namespace Fusion
{
	bool IsDriverLoaded()
	{
		SceKernelStat stat;
		return sceKernelStat(DRIVER_PATH, &stat) == 0;
	}

	std::unique_ptr<FusionDriverInfo> GetDriverInfo()
	{
		auto info = std::make_unique<FusionDriverInfo>();

		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return info;
		}

		// Make the request using ioctl.
		int res = ioctl(fd, FUSION_DRIVERINFO, info.get());
		if (res != 0)
		{
			klog("FUSION_DRIVERINFO failed with %d\n", res);

			sceKernelClose(fd);
			return info;
		}

		// tidy up.
		sceKernelClose(fd);

		return info;
	}

	std::unique_ptr<ProcInfoExt> GetProcessInfo(int processId)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return nullptr;
		}

		// Set up the args.
		Input_ProcInfo input;
		input.ProcessId = processId;

		// Allocate memory for the result
		auto tempInfo = std::make_unique<ProcInfoExt>();
		input.Output = tempInfo.get();

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_INFO, &input);
		if (res != 0)
		{
			klog("PROC_INFO failed with %d\n", res);
			sceKernelClose(fd);
			return nullptr;
		}

		// Close the driver fd immediately after using it.
		sceKernelClose(fd);

		// Return the result
		return tempInfo;
	}

	int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		OrbisLibraryInfo librariesTemp[256];
		int libCount = 0;

		Input_LibraryList input;
		input.ProcessId = processId;
		input.LibraryListOut = (OrbisLibraryInfo*)&librariesTemp;
		input.LibraryCount = &libCount;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_MODULE_LIST, &input);
		if (res != 0)
		{
			klog("PROC_MODULE_LIST failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		if (libCount > 0)
			memcpy(libraries, librariesTemp, sizeof(OrbisLibraryInfo) * maxCount);

		return libCount;
	}

	int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_ReadWriteMemory input;
		input.ProcessId = processId;
		input.ProcessAddress = addr;
		input.DataAddress = data;
		input.Length = len;
		input.IsWrite = write;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_READ_WRITE_MEMORY, &input);
		if (res != 0)
		{
			klog("PROC_READ_WRITE_MEMORY failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_AllocMemory input;
		input.ProcessId = processId;
		input.OutAddress = outAddress;
		input.Length = length;
		input.Protection = protection;
		input.Flags = flags;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_ALLOCATE_MEMORY, &input);
		if (res != 0)
		{
			klog("PROC_ALLOCATE_MEMORY failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	int FreeMemory(int processId, uint64_t processAddress, size_t length)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_FreeMemory input;
		input.ProcessId = processId;
		input.ProcessAddress = processAddress;
		input.Length = length;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_FREE_MEMORY, &input);
		if (res != 0)
		{
			klog("PROC_FREE_MEMORY failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		Input_StartThreadInfo input;
		input.ProcessId = processId;
		input.ThreadEntry = threadEntry;
		input.StackMemory = stackMemory;
		input.StackSize = stackSize;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_START_THREAD, &input);
		if (res != 0)
		{
			klog("PROC_START_THREAD failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	uint64_t Resolve(int processId, const char* library, const char* symbol)
	{
		uint64_t result = 0;
		int handle = -1;

		// Walk the module list find our module.
		OrbisLibraryInfo libraries[255];
		int realCount = GetLibraryList(processId, libraries, 255);
		for (int i = 0; i < realCount; i++)
		{
			if (strstr(libraries[i].Path, library))
			{
				handle = libraries[i].Handle;
				break;
			}
		}

		// If we did not get a handle abort now.
		if (handle == -1)
		{
			klog("Resolve(): Failed to find the handle.\n");
			return -1;
		}

		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return result;
		}

		Input_ResolveInfo input;
		input.ProcessId = processId;
		input.handle = handle;
		strcpy(input.Symbol, symbol);
		input.Result = &result;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_RESOLVE, &input);
		if (res != 0)
		{
			klog("PROC_START_THREAD failed with %d\n", res);

			sceKernelClose(fd);
			return result;
		}

		// tidy up.
		sceKernelClose(fd);

		// Return the address.
		return result;
	}

	int Jailbreak(int processId, JailBackup* backup, uint64_t authId, bool nullSandboxPath)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_Jailbreak input;
		input.ProcessId = processId;
		input.Jail = backup;
		input.AuthId = authId;
		input.NullRandPath = nullSandboxPath;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_JAILBREAK, &input);
		if (res != 0)
		{
			klog("PROC_JAILBREAK failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	int RestoreJail(int processId, JailBackup backup)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_RestoreJail input;
		input.ProcessId = processId;
		input.Jail = backup;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_JAIL, &input);
		if (res != 0)
		{
			klog("PROC_JAIL failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}

	int GetSandboxPath(int processId, char* buffer, size_t bufferSize)
	{
		// Open the kernel driver fd.
		int fd = sceKernelOpen(DRIVER_PATH, 0, 0);
		if (fd < 0)
		{
			klog("Failed to open the driver device. Do we have perms?\n");
			return -1;
		}

		// Set up the args.
		Input_SandboxPath input;
		input.ProcessId = processId;
		input.Buffer = buffer;
		input.BufferSize = bufferSize;

		// Make the request using ioctl.
		int res = ioctl(fd, PROC_SANDBOX_PATH, &input);
		if (res != 0)
		{
			klog("PROC_SANDBOX_PATH failed with %d\n", res);

			sceKernelClose(fd);
			return res;
		}

		// tidy up.
		sceKernelClose(fd);

		return 0;
	}
}