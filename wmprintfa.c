#include <winstrct.h>

EXTERN_C LPSTR
CDECL
mprintfA(LPCSTR lpMsg, ...)
{
    va_list param_list;
    LPSTR lpBuf = NULL;

    va_start(param_list, lpMsg);

    if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING, lpMsg, 0, 0, (LPSTR)&lpBuf,
        0, &param_list))
    {
        return NULL;
    }
    else
    {
        return lpBuf;
    }
}
