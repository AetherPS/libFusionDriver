#pragma once

namespace Fusion
{
	int StartThread(int processId, uint64_t entry);
	int LoadModule(int processId, char* path);
	int UnLoadModule(int processId, int handle);
}