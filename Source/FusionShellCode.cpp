#include "stdafx.h"
#include "FusionShellCode.h"
#include "FusionDriver.h"

namespace Fusion
{
	int StartThread(int processId, uint64_t entry)
	{
		Input_ShellCodeThread input;
		input.ProcessId = processId;
		input.ThreadEntry = entry;

		return MakeDriverRequest(SHELLCODE_THREAD_START, &input);
	}

	int LoadModule(int processId, char* path)
	{
		Input_LoadModule input;
		input.ProcessId = processId;
		strncpy(input.Path, path, 4096);

		int res = MakeDriverRequest(SHELLCODE_LOAD_MODULE, &input);
		if (res != 0)
		{
			return res;
		}

		return input.Result;
	}

	int UnLoadModule(int processId, int handle)
	{
		Input_UnloadModule input;
		input.ProcessId = processId;
		input.Handle = handle;

		int res = MakeDriverRequest(SHELLCODE_UNLOAD_MODULE, &input);
		if (res != 0)
		{
			return res;
		}

		return input.Result;
	}
}