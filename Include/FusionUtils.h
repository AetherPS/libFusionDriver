#pragma once

namespace Fusion
{
	void klog(const char* fmt, ...);
	void hexdump(void* ptr, int buflen, bool showAddress);
}