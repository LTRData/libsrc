#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

EXTERN_C BOOL BufSend(HANDLE h, const void *pBuf, DWORD dwBufSize,
		      DWORD dwTimeout)
{
  WOverlappedIOC ol;
  if (!ol)
    return FALSE;

  return ol.BufSend(h, pBuf, dwBufSize, dwTimeout);
}

/// Write fixed number of bytes.
BOOL
WOverlappedIOC::BufSend(HANDLE h, const void *pBuf, DWORD dwBufSize,
			DWORD dwTimeout)
{
  // First try non-overlapped. If ERROR_INVALID_PARAMETER, try overlapped.
  DWORD dwDone = 0;
  for (const void *ptr = pBuf; ; )
    {
      DWORD dwWriteLen;
      if (!WriteFile(h, ptr, dwBufSize-dwDone, &dwWriteLen, NULL))
	break;

      if (dwWriteLen == 0)
	return dwDone == dwBufSize;

      dwDone += dwWriteLen;
      *(LPBYTE*)&ptr += dwWriteLen;
      if (dwDone >= dwBufSize)
	return dwDone == dwBufSize;
    }

  dwDone = 0;
  for (const void *ptr = pBuf; dwDone < dwBufSize; )
    {
      if (!Write(h, ptr, dwBufSize-dwDone))
	break;

      if (!Wait(dwTimeout))
	break;

      DWORD dwWriteLen;
      if (!GetResult(&dwWriteLen))
	break;

      if (dwWriteLen == 0)
	break;

      dwDone += dwWriteLen;
      *(CONST BYTE**)&ptr += dwWriteLen;
    }

  return dwDone == dwBufSize;
}

