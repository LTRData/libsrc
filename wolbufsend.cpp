#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

BOOL
WOverlapped::BufSend(HANDLE hFile, const void *pBuf, DWORD dwBufSize,
		     DWORD dwTimeout)
{
  DWORD dwDone = 0;
  for (const void *ptr = pBuf; dwDone < dwBufSize; )
    {
      if (!Write(hFile, ptr, dwBufSize - dwDone))
	if (GetLastError() != ERROR_IO_PENDING)
	  break;
	else
	  if (!Wait(dwTimeout))
	    break;

      DWORD dwWriteLen;
      if (!GetResult(hFile, &dwWriteLen))
	break;

      if (dwWriteLen == 0)
	break;

      dwDone += dwWriteLen;
      *(CONST BYTE**) &ptr += dwWriteLen;
    }

  return dwDone == dwBufSize;
}

