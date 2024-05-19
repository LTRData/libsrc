#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <spsleep.h>

#ifdef _M_IX86

// This function gives away rest of current timeslice to other ready threads,
// if there is only one virtual processor in the system. Otherwise, this
// function does nothing.
void
__stdcall
SleepProcessor()
{
    Sleep(0);
}

void
__declspec(naked)
__stdcall
Dummy()
{
    __asm ret;
}

void
__stdcall
InitializeYieldSingleProcessor()
{
    SYSTEM_INFO sysinfo = { 0 };
    GetSystemInfo(&sysinfo);

    if (sysinfo.dwNumberOfProcessors >= 2)
    {
        YieldSingleProcessor = Dummy;
        return;
    }

    YieldSingleProcessor = SleepProcessor;
    Sleep(0);
}

void(__stdcall *YieldSingleProcessor)() = InitializeYieldSingleProcessor;

#endif
