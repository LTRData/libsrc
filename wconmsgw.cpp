#define WIN32_LEAN_AND_MEAN

#include <winstrct.h>

EXTERN_C
int
WINAPI
ConsoleMessageW(
    HWND,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType)
{
    FILE *stream;

    switch (uType & MB_ICONMASK)
    {
    case MB_ICONASTERISK:
    case MB_ICONHAND:
    case MB_SERVICE_NOTIFICATION:
        stream = stderr;
        break;

    default:
        stream = stdout;
    }

    LPCWSTR prompt = NULL;
    LPCWSTR buttons = NULL;

    switch (uType & MB_TYPEMASK)
    {
    case MB_CANCELTRYCONTINUE:
        prompt = L"(C)ancel, (T)ry again, Contin(u)e?";
        buttons = L"CTU";
        break;

    case MB_RETRYCANCEL:
        prompt = L"(R)etry, (C)ancel?";
        buttons = L"RC";
        break;

    case MB_YESNO:
        prompt = L"(Y)es, (N)o?";
        buttons = L"YN";
        break;

    case MB_YESNOCANCEL:
        prompt = L"(Y)es, (N)o, (C)ancel?";
        buttons = L"YNC";
        break;

    case MB_ABORTRETRYIGNORE:
        prompt = L"(A)bort, (R)etry, (I)gnore?";
        buttons = L"ARI";
        break;

    case MB_OKCANCEL:
        prompt = L"(O)K, (C)ancel?";
        buttons = L"OC";
        break;
    }

    fwprintf(stream,
        L"\r\n"
        L" -- %s --\r\n"
        L"\n"
        L"%s\r\n",
        lpCaption ? lpCaption : L"***", lpText);

    fflush(stream);

    if (prompt == NULL || buttons == NULL)
    {
        return IDOK;
    }

    UINT defbtn = (uType & MB_DEFMASK) >> 8;
    if (defbtn >= wcslen(buttons))
    {
        defbtn = 0;
    }

    wchar_t answer;
    for (;;)
    {
        fwprintf(stream,
            L"%s %c\b",
            prompt, buttons[defbtn]);

        fflush(stream);

        answer = towupper(_fgetwchar());

        if (answer == '\r' || answer == '\n' || answer == 0)
        {
            answer = buttons[defbtn];
        }
        else if (wcschr(buttons, answer) == NULL)
        {
            continue;
        }

        switch (answer)
        {
        case 'O':
            return IDOK;

        case 'C':
            return IDCANCEL;

        case 'A':
            return IDABORT;

        case 'R':
            return IDRETRY;

        case 'I':
            return IDIGNORE;

        case 'Y':
            return IDYES;

        case 'N':
            return IDNO;

        case 'T':
            return IDTRYAGAIN;

        case 'U':
            return IDCONTINUE;
        }
    }
}
