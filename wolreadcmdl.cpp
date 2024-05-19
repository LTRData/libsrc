#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

/* Read until end-of-line and optionally echo typed characters to hEchoDevice.
 * Set hEchoDevice to INVALID_HANDLE_VALUE to turn off echoing.
 * This function supports backspace functionality. dwTimeOut is the timeout
 * in ms, set to INFINITE to disable timeouting.
 */
EXTERN_C BOOL ReadCommandLine(HANDLE hConn, LPSTR lpBuffer, DWORD dwBufferSize,
			      HANDLE hEcho, DWORD dwTimeout)
{
  WOverlapped ol;
  if (!ol)
    return FALSE;

  return ol.ReadCommandLine(hConn, lpBuffer, dwBufferSize, hEcho, dwTimeout);
}

BOOL
WOverlapped::ReadCommandLine(HANDLE hConn, LPSTR lpBuffer, DWORD dwBufferSize,
			     HANDLE hEcho, DWORD dwTimeout)
{
  char *pBufptr = lpBuffer;

  for(;;)
    {
      Sleep(0);

      // Correction for Win 95 ReadFile() does not reset the event.
      ResetEvent();

      DWORD dwReadLen;
      if (!ReadFile(hConn, pBufptr, 1, &dwReadLen, this))
	if (GetLastError() != ERROR_IO_PENDING)
	  return false;
	else
	  if (!Wait(dwTimeout))
	    {
	      SetLastError(ERROR_TIMEOUT);
	      return false;
	    }

      if (!GetResult(hConn, &dwReadLen))
	return false;

      if (dwReadLen != 1)
	{
	  SetLastError(ERROR_HANDLE_EOF);
	  return false;
	}

      if (*pBufptr == '\n')
	break;

      switch (*pBufptr)
	{
	case '\r':
	  continue;
	case 8: case 127:
	  if (pBufptr > lpBuffer)
	    {
	      char cBackspaceBack[] = { 8, ' ', 8 };
	      if (hEcho != INVALID_HANDLE_VALUE)
		if (!BufSend(hEcho, cBackspaceBack, sizeof cBackspaceBack))
		  return false;

	      --pBufptr;
	    }
	  break;
	case 0:
	  break;
	default:
	  if (pBufptr >= lpBuffer+dwBufferSize)
	    continue;

	  if (hEcho != INVALID_HANDLE_VALUE)
	    if (!BufSend(hEcho, pBufptr, 1))
	      return false;

	  ++pBufptr;
	}
    }

  *pBufptr = NULL;

  if (hEcho != INVALID_HANDLE_VALUE)
    if (!StrSend(hEcho, "\r\n"))
      return false;

  Sleep(0);
  return true;
}

