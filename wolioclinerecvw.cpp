#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

/* This one is like LineRecv() but receives a line with Unicode characters.
 */
EXTERN_C DWORD
LineRecvW(HANDLE h, LPWSTR pBuf, DWORD dwBufSize, DWORD dwTimeout)
{
  WOverlappedIOC ol;
  if (!ol)
    return 0;

  return ol.LineRecvW(h, pBuf, dwBufSize, dwTimeout);
}

DWORD
WOverlappedIOC::LineRecvW(HANDLE h, LPWSTR pBuf, DWORD dwBufChrs,
			  DWORD dwTimeout)
{
  if (dwBufChrs < 2)
    return 0;

  bool bGood = true;
  DWORD dwLen = 0;
  for (LPWSTR ptr = pBuf; dwLen < dwBufChrs - 2; )
    {
      DWORD dwReadLen;
      if (!ReadFile(h, ptr, sizeof WCHAR, &dwReadLen, NULL))
	{
	  bGood = false;
	  break;
	}

      if (dwReadLen != sizeof WCHAR)
	{
	  bGood = false;
	  SetLastError(ERROR_HANDLE_EOF);
	  break;
	}

      if (ptr[0] == L'\r')
	continue;

      if (ptr[0] == L'\n')
	break;

      dwLen ++;
      ptr ++;
    }

  if (bGood && (dwLen == 0))
    SetLastError(ERROR_SUCCESS);

  if (bGood)
    {
      pBuf[dwLen] = 0;
      return dwLen;
    }

  if (GetLastError() != ERROR_INVALID_PARAMETER)
    return 0;

  dwLen = 0;
  bGood = true;
  for (LPWSTR ptr = pBuf; dwLen < dwBufChrs - 2; )
    {
      if (!Read(h, ptr, sizeof(WCHAR)))
	{
	  bGood = false;
	  break;
	}

      if (!Wait(dwTimeout))
	{
	  bGood = false;
	  break;
	}

      DWORD dwReadLen;
      if (!GetResult(&dwReadLen))
	{
	  bGood = false;
	  break;
	}

      if (dwReadLen != sizeof(WCHAR))
	{
	  bGood = false;
	  SetLastError(ERROR_HANDLE_EOF);
	  break;
	}

      if (ptr[0] == L'\r')
	continue;

      if (ptr[0] == L'\n')
	break;

      dwLen ++;
      ptr ++;
    }

  if (bGood & (dwLen == 0))
    SetLastError(ERROR_SUCCESS);

  pBuf[dwLen] = 0;
  return dwLen;
}
