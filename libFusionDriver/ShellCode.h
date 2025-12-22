#pragma once

#define STACK_SIZE 0x80000

namespace Fusion
{
	struct ThreadShellCodeHeader
	{
		uint32_t magic;
		uint64_t entry;

		uint64_t thr_initial;
		uint64_t scePthreadCreate;
		uint64_t scePthreadJoin;
		uint64_t scePthreadExit;
		uint64_t ThreadEntry;
		uint8_t ShellCodeComplete;
	}__attribute__((packed));

	struct SprxLoaderHeader
	{
		uint32_t magic;
		uint64_t entry;

		uint64_t sceKernelLoadStartModule;
		uint64_t scePthreadExit;

		int32_t ModuleHandle;
		char Path[4096];
	}__attribute__((packed));

	struct SprxUnLoaderHeader
	{
		uint32_t magic;
		uint64_t entry;

		uint64_t sceKernelStopUnloadModule;
		uint64_t scePthreadExit;

		int32_t ModuleHandle;
		int32_t Result;
	}__attribute__((packed));

	int StartPThread(int processId, uint64_t entryPoint);
	int LoadSprx(int processId, const char* path);
	int UnloadSprx(int processId, int handle);
}