#pragma once

namespace Fusion
{
	int LoadSprx(int processId, const std::string& path, int* outResult = nullptr);
	int UnloadSprx(int processId, int moduleHandle, int* outResult = nullptr);
}