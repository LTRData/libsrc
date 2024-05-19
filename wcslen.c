#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#ifdef _DLL
#pragma comment(lib, "minwcrt")
#endif

size_t
__declspec(naked) wcslen(LPCWSTR lp)
{
  __asm
    {
      mov edi,[esp+4] ;
      mov ax,0 ;
      mov ecx,0xffffffff ;
      repne scasw ;
      mov eax,0xfffffffe ;
      sub eax,ecx ;
      ret ;
    }
}

int wmain(int argc, LPWSTR *argv)
{
  WCHAR a[] = L"ABCDEF";
  wcscpy(a, argv[0]);
  printf("'%s'\n", a);
}
