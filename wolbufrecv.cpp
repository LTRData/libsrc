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
WOverlapped::BufRecv(HANDLE hFile, PVOID pBuf, DWORD dwBufSize,
		     DWORD dwTimeout)
{
  DWORD dwDone = 0;
  bool bGood = true;

  for (PVOID ptr = pBuf; dwDone < dwBufSize; )
    {
      if (!Read(hFile, ptr, dwBufSize - dwDone))
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

      if (dwReadLen == 0)
	break;

      dwDone += dwReadLen;
      (*(LPBYTE*) &ptr) += dwReadLen;
    }

  if (bGood & (dwDone != dwBufSize))
    SetLastError(ERROR_HANDLE_EOF);

  return dwDone;
}

