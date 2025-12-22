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

	int StartPThread(int processId, uint64_t entryPoint);
}