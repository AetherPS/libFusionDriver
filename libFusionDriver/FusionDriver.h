#pragma once
#include "DriverDefinitions.h"
#include <memory>

#define DRIVER_PATH "/dev/Fusion"

namespace Fusion
{
	bool IsDriverLoaded();

	int GetDriverInfo(FusionDriverInfo* info);

	int Jailbreak(int processId, JailBackup* backup, uint64_t authId = -1, bool nullSandboxPath = false);
	int RestoreJail(int processId, JailBackup backup);
	int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount, int* realCount);
	int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write);
	int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags);
	int FreeMemory(int processId, uint64_t processAddress, size_t length);
	int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize);
	int Resolve(int processId, int libHandle, const char* library, const char* symbol, unsigned int flags, uint64_t* addr);
	int GetAuthId(int processId, uint64_t* authId);
	int SetAuthId(int processId, uint64_t authId);

	// Helpful wrappers.
	uint64_t GetRemoteAddress(int processId, const char* library, uint64_t offset);
	int GetLibraryHandle(int processId, const char* library);
}