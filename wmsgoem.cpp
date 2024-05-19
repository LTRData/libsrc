#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winstrct.hpp>

WMsgOEM::WMsgOEM(WMem<WCHAR> &mem)
{
    int i = WideCharToMultiByte(CP_OEMCP, 0, mem,
        (DWORD)mem.Count(), NULL, 0, NULL, NULL);

    ptr = (LPSTR)halloc_seh(i);
    if (WideCharToMultiByte(CP_OEMCP, 0, (LPWSTR)mem, (int)mem.GetSize() >> 1,
        (LPSTR)ptr, i, NULL, NULL) == i)
        return;

    Free();
}
