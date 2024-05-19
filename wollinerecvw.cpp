#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

DWORD
WOverlapped::LineRecvW(HANDLE hFile, LPWSTR pBuf, DWORD dwBufSize,
		       DWORD dwTimeout)
{
  if (dwBufSize < 2)
    return 0;

  bool bGood = true;
  DWORD dwLen = 0;
  for (LPWSTR ptr = pBuf; dwLen < dwBufSize - 2; )
    {
      if (!Read(hFile, ptr, sizeof WCHAR))
	if (GetLastError() != ERROR_IO_PENDING)
	  {
	    bGood = false;
	    break;
	  }
	else
	  if (!Wait(dwTimeout))
	    {
	      bGood = false;
	      break;
	    }

      DWORD dwReadLen;
      if (!GetResult(hFile, &dwReadLen))
	{
	  bGood = false;
	  break;
	}

      if (dwReadLen != sizeof WCHAR)
	break;

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
      pBuf[dwLen] = '\x00';
      return dwLen;
    }

  if (GetLastError() == ERROR_HANDLE_EOF)
    {
      pBuf[dwLen] = '\x00';
      return dwLen;
    }

  return 0;
}

