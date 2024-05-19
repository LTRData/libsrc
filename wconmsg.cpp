#define WIN32_LEAN_AND_MEAN

#include <winstrct.h>

EXTERN_C
int
WINAPI
ConsoleMessageA(
    HWND,
    LPCSTR lpText,
    LPCSTR lpCaption,
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
    
    LPCSTR prompt = NULL;
    LPCSTR buttons = NULL;

    switch (uType & MB_TYPEMASK)
    {
    case MB_CANCELTRYCONTINUE:
        prompt = "(C)ancel, (T)ry again, Contin(u)e?";
        buttons = "CTU";
        break;

    case MB_RETRYCANCEL:
        prompt = "(R)etry, (C)ancel?";
        buttons = "RC";
        break;

    case MB_YESNO:
        prompt = "(Y)es, (N)o?";
        buttons = "YN";
        break;

    case MB_YESNOCANCEL:
        prompt = "(Y)es, (N)o, (C)ancel?";
        buttons = "YNC";
        break;

    case MB_ABORTRETRYIGNORE:
        prompt = "(A)bort, (R)etry, (I)gnore?";
        buttons = "ARI";
        break;

    case MB_OKCANCEL:
        prompt = "(O)K, (C)ancel?";
        buttons = "OC";
        break;
    }

    fprintf(stream,
        "\r\n"
        " -- %s --\r\n"
        "\n"
        "%s\r\n",
        lpCaption ? lpCaption : "***", lpText);

    fflush(stream);

    if (prompt == NULL || buttons == NULL)
    {
        return IDOK;
    }

    UINT defbtn = (uType & MB_DEFMASK) >> 8;
    if (defbtn >= strlen(buttons))
    {
        defbtn = 0;
    }

    int answer;
    for (;;)
    {
        fprintf(stream,
            "%s %c\b",
            prompt, buttons[defbtn]);

        fflush(stream);

        answer = toupper(_fgetchar());

        if (answer == '\r' || answer == '\n' || answer == 0)
        {
            answer = buttons[defbtn];
        }
        else if (strchr(buttons, answer) == NULL)
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
