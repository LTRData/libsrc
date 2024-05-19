#define WIN32_LEAN_AND_MEAN

#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

/* Read a block from an overlapped device, such as communication port or
 * socket. Returns the number of characters written in buffer.
 *
 * If return value is different from dwBufSize parameter, GetLastError()
 * returns ERROR_HANDLE_EOF if reached EOF.
 */
EXTERN_C DWORD BufRecv(HANDLE h, void *pBuf, DWORD dwBufSize, DWORD dwTimeout)
{
  WOverlappedIOC ol;
  if (!ol)
    return 0;

  return ol.BufRecv(h, pBuf, dwBufSize, dwTimeout);
}

DWORD
WOverlappedIOC::BufRecv(HANDLE h, PVOID pBuf, DWORD dwBufSize, DWORD dwTimeout)
{
  // First try non-overlapped. If ERROR_INVALID_PARAMETER, try overlapped.
  DWORD dwDone = 0;
  for (void *ptr = pBuf; ; )
    {
      DWORD dwReadLen;
      if (!ReadFile(h, ptr, dwBufSize-dwDone, &dwReadLen, NULL))
	if (GetLastError() == ERROR_INVALID_PARAMETER)
	  break;
	else
	  return 0;

      if (dwReadLen == 0)
	break;

      dwDone += dwReadLen;
      (*(LPBYTE*)&ptr) += dwReadLen;
      if (dwDone >= dwBufSize)
	return dwDone;
    }

  dwDone = 0;
  bool bGood = true;

  for (PVOID ptr = pBuf; dwDone < dwBufSize; )
    {
      if (!Read(h, ptr, dwBufSize-dwDone))
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

      if (dwReadLen == 0)
	break;

      dwDone += dwReadLen;
      (*(LPBYTE*)&ptr) += dwReadLen;
    }

  if (bGood & (dwDone != dwBufSize))
    SetLastError(ERROR_HANDLE_EOF);

  return dwDone;
}
