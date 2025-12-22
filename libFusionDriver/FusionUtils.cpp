#include "stdafx.h"
#include "FusionUtils.h"
#include "FusionDriver.h"

namespace Fusion
{
	void klog(const char* fmt, ...)
	{
		char Buffer[0x200];

		//Create full string from va list.
		va_list args;
		va_start(args, fmt);
		vsprintf(Buffer, fmt, args);
		va_end(args);

		sceKernelDebugOutText(0, Buffer);
	}

	void hexdump(void* ptr, int buflen, bool showAddress)
	{
		unsigned char* buf = (unsigned char*)ptr;
		int i, j;
		for (i = 0; i < buflen; i += 16)
		{
			showAddress ? klog("%06x: ", (uint64_t)ptr + i) : klog("%06x: ", i);
			for (j = 0; j < 16; j++)
				if (i + j < buflen)
					klog("%02x ", buf[i + j]);
				else
					klog("   ");
			klog(" ");
			for (j = 0; j < 16; j++)
				if (i + j < buflen)
					klog("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
			klog("\n");
		}
	}
}