#include <winstrct.h>

EXTERN_C LPWSTR
CDECL
mprintfW(LPCWSTR lpMsg, ...)
{
    va_list param_list;
    LPWSTR lpBuf = NULL;

    va_start(param_list, lpMsg);

    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING, lpMsg, 0, 0, (LPWSTR)&lpBuf,
        0, &param_list))
    {
        return NULL;
    }
    else
    {
        return lpBuf;
    }
}

