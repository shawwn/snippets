#include "MemoryTracker.h"

#ifdef WIN32
#include <windows.h>
#endif

NVSHARE::MemoryTracker *gMemoryTracker=0;

namespace NVSHARE
{

void createMemoryTracker(const char *memoryTrackerDLL)
{
#ifdef WIN32
	if ( gMemoryTracker == 0 && memoryTrackerDLL )
	{
		UINT errorMode = SEM_FAILCRITICALERRORS;
        UINT oldErrorMode = SetErrorMode(errorMode);
        HMODULE module = LoadLibraryA(memoryTrackerDLL);
        SetErrorMode(oldErrorMode);
        void *proc = GetProcAddress(module,"getInterface");
        if ( proc )
        {
          typedef void * (__cdecl * NX_GetToolkit)(int version,void *systemServices);
          gMemoryTracker = (NVSHARE::MemoryTracker *)((NX_GetToolkit)proc)(MEMORY_TRACKER_VERSION,0);
        }
	}
#endif
}

};
