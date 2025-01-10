#pragma once
#include "DriverDefinitions.h"

#define DRIVER_PATH "/dev/Fusion"

namespace Fusion
{
	bool IsDriverLoaded();
	std::unique_ptr<FusionDriverInfo> GetDriverInfo();

	std::unique_ptr<ProcInfoExt> GetProcessInfo(int processId);
	int GetLibraryList(int processId, OrbisLibraryInfo* libraries, int maxCount);
	int ReadWriteMemory(int processId, uint64_t addr, void* data, size_t len, bool write);
	int AllocateMemory(int processId, uint64_t* outAddress, size_t length, int protection, int flags);
	int FreeMemory(int processId, uint64_t processAddress, size_t length);
	int StartThread(int processId, uint64_t threadEntry, uint64_t stackMemory, size_t stackSize);
	uint64_t Resolve(int processId, const char* library, const char* symbol);
	int Jailbreak(int processId, JailBackup* backup, uint64_t authId = -1, bool nullSandboxPath = false);
	int RestoreJail(int processId, JailBackup backup);
	int GetSandboxPath(int processId, char* buffer, size_t bufferSize);
}