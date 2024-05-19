#define WIN32_LEAN_AND_MEAN
#include <winstrct.h>
#include <stdio.h>

#pragma comment(lib, "user32")

void win_perrorA(LPCSTR __errmsg)
{
    LPSTR errmsg = NULL;
    DWORD errcode = GetLastError();

    FormatMessageA(FORMAT_MESSAGE_MAX_WIDTH_MASK |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errmsg, 0, NULL);

    if (__errmsg ? !!*__errmsg : FALSE)
        if (errmsg != NULL)
            oem_printf(stderr, "%1: %2%%n", __errmsg, errmsg);
        else
            fprintf(stderr, "%s: Win32 error %u\n", __errmsg, (DWORD)errcode);
    else
        if (errmsg != NULL)
            oem_printf(stderr, "%1%%n", errmsg);
        else
            fprintf(stderr, "Win32 error %u\n", (DWORD)errcode);

    if (errmsg != NULL)
        LocalFree(errmsg);
}
