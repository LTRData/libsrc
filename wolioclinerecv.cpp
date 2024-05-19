#define WIN32_LEAN_AND_MEAN
#include <wio.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32")
#endif

//-- wio.h definitions
//--------------- SafeSend(), LineRecv(), BufRecv() and BufSend() functions are
//--------------- used for I/O on overlapped streams such as sockets, comm
//--------------- devices or named pipes.

/* Read a line from a file or device. It first tries non-overlapped read in
 * the device is a file and the read operation should start from the current
 * position. If non-overlapped read is not supported, such as communication
 * port or socket it tries overlapped read.
 * Returns the number of characters written in buffer. The newline character is
 * not included in the buffer or in the return value, but is received from the
 * stream.
 * If zero is returned, GetLastError() will return an error code if an error
 * occured or ERROR_SUCCESS if just a blank line was received.
 */
EXTERN_C DWORD
LineRecv(HANDLE h, LPSTR pBuf, DWORD dwBufSize, DWORD dwTimeout)
{
  WOverlappedIOC ol;
  if (!ol)
    return 0;

  return ol.LineRecv(h, pBuf, dwBufSize, dwTimeout);
}

DWORD
WOverlappedIOC::LineRecv(HANDLE h, LPSTR pBuf, DWORD dwBufSize,
			 DWORD dwTimeout)
{
  if (dwBufSize < (sizeof(CHAR)*2))
    return 0;

  bool bGood = true;
  DWORD dwLen = 0;
  for (LPSTR ptr = pBuf; dwLen < dwBufSize - (sizeof(CHAR)*2); )
    {
      DWORD dwReadLen;
      if (!ReadFile(h, ptr, sizeof CHAR, &dwReadLen, NULL))
	{
	  bGood = false;
	  break;
	}

      if (dwReadLen != sizeof CHAR)
	{
	  bGood = false;
	  SetLastError(ERROR_HANDLE_EOF);
	  break;
	}

      if (ptr[0] == '\r')
	continue;

      if (ptr[0] == '\n')
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

  switch (GetLastError())
    {
    case ERROR_HANDLE_EOF:
      pBuf[dwLen] = '\x00';
      return dwLen;

    case ERROR_INVALID_PARAMETER:
    case ERROR_INVALID_FUNCTION:
      break;

    default:
      return 0;
    }

  dwLen = 0;
  bGood = true;
  for (char *ptr = pBuf; dwLen < dwBufSize - (sizeof(CHAR)*2); )
    {
      if (!Read(h, ptr, sizeof(CHAR)))
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

      if (dwReadLen != sizeof(CHAR))
	{
	  bGood = false;
	  SetLastError(ERROR_HANDLE_EOF);
	  break;
	}

      if (ptr[0] == '\r')
	continue;

      if (ptr[0] == '\n')
	break;

      dwLen ++;
      ptr ++;
    }

  if (bGood & (dwLen == 0))
    SetLastError(ERROR_SUCCESS);

  pBuf[dwLen] = '\x00';
  return dwLen;
}

