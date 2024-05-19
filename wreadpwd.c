#include <winstrct.h>
#include <wio.h>

EXTERN_C DWORD
ReadPassword(HANDLE h, LPSTR pBuf, DWORD dwBufSiz)
{
    char *ptr = pBuf;

    DWORD mode;

    BOOL got_mode = GetConsoleMode(h, &mode);

    if (got_mode)
    {
        SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    }

    while (ptr < pBuf + dwBufSiz - 2)
    {
        DWORD dwReadLen;

        if (!ReadFile(h, ptr, 1, &dwReadLen, NULL))
        {
            *ptr = '\x00';
            break;
        }

        if (dwReadLen != 1)
        {
            *ptr = '\x00';
            break;
        }

        if ((*ptr == '\r') | (*ptr == '\n'))
        {
            *ptr = '\x00';
            break;
        }

        if (*ptr == 8)
        {
            if (ptr > pBuf)
            {
                fputs("\x08 \x08", stdout);
                --ptr;
            }
            continue;
        }

        fputc('*', stdout);
        ++ptr;
    }

    if (got_mode)
    {
        SetConsoleMode(h, mode);
    }

    return (DWORD)(ptr - pBuf);
}
