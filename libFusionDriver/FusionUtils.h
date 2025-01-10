#pragma once

namespace Fusion
{
	void klog(const char* fmt, ...);
	void hexdump(void* ptr, int buflen, bool showAddress);
	uint64_t GetRemoteAddress(int processId, const char* library, uint64_t offset);
}