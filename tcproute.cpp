#define BUILDING_INETDFWD_DLL

#include <winstrct.h>
#include <wsocket.hpp>
#include <wio.h>
#include <wtime.h>

#include <tcproute.hpp>

#define SetSockOpts(s)

#if TCP_PACKET_SIZE < 266

#pragma message("Gateway functions won't compile with packet size < 266 bytes")

#else // TCP_PACKET_SIZE >= 266

SOCKET
ConnectTCPHttpProxy(LPCSTR szServer, LPCSTR szService)
{
    const char *argv[] = {
        getenv("TCPROUTE_PROXYADDRESS"),
        getenv("TCPROUTE_PROXYPORT"),
        NULL
    };

    if (argv[0] == NULL)
    {
        LogErr("ConnectTCPHttpProxy: TCPROUTE_PROXYADDRESS is not in "
            "environment!\n");
        return (DWORD)-1;
    }

    if (argv[1] == NULL)
        argv[1] = "8080";

    if (szService == NULL)
        szService = "23";

    LogMsg("Connecting to %s:%s...\n", argv[0], argv[1]);
    SOCKET sRemote = ConnectTCP(argv[0], argv[1]);
    if (sRemote == INVALID_SOCKET)
    {
        LogErr("ConnectTCPHttpProxy: Connection to proxy failed.\n");
        return (DWORD)-1;
    }
    LogMsg("Connected to %s:%s.\n", argv[0], argv[1]);

    SetSockOpts(sRemote);

    WHeapMem<CHAR> pPacket(TCP_PACKET_SIZE);
    if (!pPacket)
    {
        LogWinErr("ConnectTCPHttpProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    WOverlappedIOC ol;
    if (!ol)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    LogMsg("Connecting proxy to %s:%s.\n",
        szServer, szService);

    ol.StrSend((HANDLE)sRemote, "CONNECT ");
    ol.StrSend((HANDLE)sRemote, szServer);
    ol.StrSend((HANDLE)sRemote, ":");
    ol.StrSend((HANDLE)sRemote, szService);
    ol.StrSend((HANDLE)sRemote, " HTTP/1.0\r\nConnection: Close\r\n\r\n");
    ol.LineRecv((HANDLE)sRemote, pPacket, TCP_PACKET_SIZE);
    puts(pPacket);
    char cOKHead[] = "HTTP/1.0 200 ";
    if (_strnicmp(pPacket, cOKHead, sizeof(cOKHead) - 1))
    {
        closesocket(sRemote);
        return (DWORD)-1;
    }

    while (ol.LineRecv((HANDLE)sRemote, pPacket, TCP_PACKET_SIZE))
    {
        puts(pPacket);
        Sleep(0);
    }

    return sRemote;
}

SOCKET
ConnectTCPHttpProxyWithLookup(LPCSTR szServer, LPCSTR szService)
{
    const char *argv[] = {
        getenv("TCPROUTE_PROXYADDRESS"),
        getenv("TCPROUTE_PROXYPORT"),
        NULL
    };

    if (argv[0] == NULL)
    {
        LogErr("ConnectTCPHttpProxy: TCPROUTE_PROXYADDRESS is not in "
            "environment!\n");
        return (DWORD)-1;
    }

    if (argv[1] == NULL)
        argv[1] = "8080";

    if (szService == NULL)
        szService = "23";

    // Get destination address if it is an IP address
    u_long haddr = inet_addr(szServer);

    // Get destination address
    if (haddr == INADDR_NONE)
    {
        hostent *hent = gethostbyname(szServer);
        if (!hent)
        {
            LogErr("ConnectTCPSocksProxyWithLookup: gethostbyname() failed "
                "for %s.\n", szServer);
            return (DWORD)-1;
        }

        haddr = *(u_long *)hent->h_addr;
    }

    sockaddr_in addrDest;
    addrDest.sin_addr.s_addr = haddr;

    LogMsg("Connecting to %s:%s...\n", argv[0], argv[1]);
    SOCKET sRemote = ConnectTCP(argv[0], argv[1]);
    if (sRemote == INVALID_SOCKET)
    {
        LogErr("ConnectTCPHttpProxy: Connection to proxy failed.\n");
        return (DWORD)-1;
    }
    LogMsg("Connected to %s:%s.\n", argv[0], argv[1]);

    SetSockOpts(sRemote);

    WHeapMem<CHAR> pPacket(TCP_PACKET_SIZE);
    if (!pPacket)
    {
        LogWinErr("ConnectTCPHttpProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    WOverlappedIOC ol;
    if (!ol)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    LogMsg("Connecting proxy to %s:%s.\n",
        inet_ntoa(addrDest.sin_addr), szService);

    ol.StrSend((HANDLE)sRemote, "CONNECT ");
    ol.StrSend((HANDLE)sRemote, inet_ntoa(addrDest.sin_addr));
    ol.StrSend((HANDLE)sRemote, ":");
    ol.StrSend((HANDLE)sRemote, szService);
    ol.StrSend((HANDLE)sRemote, " HTTP/1.0\r\nConnection: Close\r\n\r\n");
    ol.LineRecv((HANDLE)sRemote, pPacket, TCP_PACKET_SIZE);
    puts(pPacket);
    char cOKHead[] = "HTTP/1.0 200 ";
    if (_strnicmp(pPacket, cOKHead, sizeof(cOKHead) - 1))
    {
        closesocket(sRemote);
        return (DWORD)-1;
    }

    while (ol.LineRecv((HANDLE)sRemote, pPacket, TCP_PACKET_SIZE))
    {
        puts(pPacket);
        Sleep(0);
    }

    return sRemote;
}

SOCKET
ConnectTCPSocksProxy(LPCSTR szServer, LPCSTR szService)
{
    const char *argv[] = {
        getenv("TCPROUTE_SOCKSADDRESS"),
        getenv("TCPROUTE_SOCKSPORT"),
        getenv("TCPROUTE_SOCKSVERSION"),
        szServer,
        szService,
        NULL
    };

    if (argv[0] == NULL)
    {
        LogErr("ConnectTCPSocksProxy: TCPROUTE_SOCKSADDRESS is not in "
            "environment!\n");
        return (DWORD)-1;
    }

    if (argv[1] == NULL)
        argv[1] = "1080";

    if (argv[2] == NULL)
        argv[2] = "4";

    if ((argv[2][0] != '4') & (argv[2][0] != '5'))
    {
        LogErr("ConnectTCPSocksProxy: Unsupported socks version level.\n");
        return (DWORD)-1;
    }

    if (argv[3] == NULL)
    {
        LogErr("ConnectTCPSocksProxy: Need a destination host and port "
            "as argument.\n");
        return (DWORD)-1;
    }

    if (strlen(argv[3]) > 254)
    {
        LogErr("ConnectTCPSocksProxy: Too long destination address.\n");
        return (DWORD)-1;
    }

    // Get destination address if it is an IP address
    u_long destaddr = inet_addr(argv[3]);
    UCHAR ucDestAddressLength =
        (destaddr == INADDR_NONE) ? (UCHAR)strlen(argv[3]) : (UCHAR)0;

    if (argv[4] == NULL)
        argv[4] = "23";

    // Get destination port
    u_short hport = htons((u_short)atol(argv[4]));
    if (hport == 0)
    {
        servent *sent = getservbyname(argv[4], "tcp");
        if (!sent)
        {
            LogErr("ConnectTCPSocksProxy: getservbyname() failed for %s.\n",
                argv[4]);
            return (DWORD)-1;
        }

        hport = sent->s_port;
    }

    LogMsg("Connecting to %s:%s...\n", argv[0], argv[1]);
    SOCKET sRemote = ConnectTCP(argv[0], argv[1]);
    if (sRemote == INVALID_SOCKET)
    {
        LogErr("ConnectTCPSocksProxy: Connection to proxy failed.\n");
        return (DWORD)-1;
    }
    LogMsg("Connected to %s:%s.\n", argv[0], argv[1]);

    SetSockOpts(sRemote);

    WHeapMem<CHAR> pPacket(TCP_PACKET_SIZE);
    if (!pPacket)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    WOverlappedIOC ol;
    if (!ol)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    pPacket[0] = (UCHAR)atoi(argv[2]);

    // If version 5, do some init
    if (pPacket[0] == '\x05')
    {
        pPacket[1] = '\x01';
        pPacket[2] = '\x00';

        LogMsg("ConnectTCPSocksProxy: Connected to %s:%s, version %i...\n",
            argv[0], argv[1], (int)pPacket[0]);

        if (!ol.BufSend((HANDLE)sRemote, (LPSTR)pPacket, 3))
        {
            LogWinErr("ConnectTCPSocksProxy: Error sending to socks server");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (ol.BufRecv((HANDLE)sRemote, (LPSTR)pPacket, 2) != 2)
        {
            LogWinErr("ConnectTCPSocksProxy: Error receiving from socks "
                "server");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (pPacket[0] == '\xFF')
        {
            LogErr("ConnectTCPSocksProxy: The socks server denied "
                "the request method.\n");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (pPacket[1] != '\x00')
        {
            LogErr("ConnectTCPSocksProxy: The socks server requires "
                "authentication.\n");
            closesocket(sRemote);
            return (DWORD)-1;
        }
    }

    LogMsg("Connecting proxy to %s:%s.\n", szServer, szService);

    int iRequestPacketLength;
    pPacket[1] = '\x01';		// CONNECT request

    // If target address was not IP address, send the domain name to the socks
    // server
    if (destaddr == INADDR_NONE)
    {
        // Connect request is a bit diff between versions
        if (pPacket[0] == '\x04')
        {
            *(u_short *)(pPacket + 2) = hport;
            pPacket[4] = '\x00';
            pPacket[5] = '\x00';
            pPacket[6] = '\x00';
            pPacket[7] = '\xFF';
            pPacket[8] = '\x00';
            strcpy(pPacket + 9, argv[3]);
            iRequestPacketLength = 9 + ucDestAddressLength + 1;
        }
        else
        {
            pPacket[2] = '\x00';
            pPacket[3] = '\x03';
            pPacket[4] = ucDestAddressLength;
            CopyMemory(pPacket + 5, argv[3], ucDestAddressLength);
            *(u_short *)(pPacket + 5 + ucDestAddressLength) = hport;

            iRequestPacketLength = 5 + ucDestAddressLength + 2;
        }
    }
    else
    {
        if (pPacket[0] == '\x04')
        {
            *(u_short *)(pPacket + 2) = hport;
            *(u_long *)(pPacket + 4) = destaddr;
            pPacket[8] = '\x00';

            iRequestPacketLength = 9;
        }
        else
        {
            pPacket[2] = '\x00';
            pPacket[3] = '\x01';
            *(u_long *)(pPacket + 4) = destaddr;
            *(u_short *)(pPacket + 8) = hport;

            iRequestPacketLength = 10;
        }
    }

    LogMsg("ConnectTCPSocksProxy: Ok, sending request for %s:%u...\n",
        argv[3], (UINT)ntohs(hport));

    if (!ol.BufSend((HANDLE)sRemote, (LPSTR)pPacket, iRequestPacketLength))
    {
        LogWinErr("ConnectTCPSocksProxy: Error sending to socks server");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    if (pPacket[0] == '\x04')
        iRequestPacketLength = 8;
    else
        iRequestPacketLength = 10;

    puts("ConnectTCPSocksProxy: Waiting for reply...");

    DWORD iLen = ol.BufRecv((HANDLE)sRemote, (LPSTR)pPacket, iRequestPacketLength);
    if ((iLen == SOCKET_ERROR) | (iLen < 7))
    {
        LogWinErr("ConnectTCPSocksProxy: Error receiving from socks server");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    if (!((pPacket[0] == 0) & (pPacket[1] == 90)) &
        !((pPacket[0] == '\x04') & (pPacket[1] == '\x00')) &
        !((pPacket[0] == '\x05') & (pPacket[1] == '\x00')))
    {
        LogErr("ConnectTCPSocksProxy: The socks server failed to connect ("
            "level %i status %i).\n", (int)pPacket[0], (int)pPacket[1]);
        closesocket(sRemote);
        return pPacket[1];
    }

    in_addr iaddr;
    iaddr.s_addr = *(u_long *)(pPacket + 4);
    if (pPacket[0] == '\x00')
        hport = *(u_short *)(pPacket + 2);
    else
        hport = *(u_short *)(pPacket + 8);

    LogMsg("ConnectTCPSocksProxy: Ok, SOCKS interface is %s:%u.\n",
        inet_ntoa(iaddr), (UINT)ntohs(hport));

    return sRemote;
}

SOCKET
ConnectTCPSocksProxyWithLookup(LPCSTR szServer, LPCSTR szService)
{
    const char *argv[] = {
        getenv("TCPROUTE_SOCKSADDRESS"),
        getenv("TCPROUTE_SOCKSPORT"),
        getenv("TCPROUTE_SOCKSVERSION"),
        szServer,
        szService,
        NULL
    };

    if (argv[0] == NULL)
    {
        LogErr("ConnectTCPSocksProxy: TCPROUTE_SOCKSADDRESS is not in "
            "environment!\n");
        return (DWORD)-1;
    }

    if (argv[1] == NULL)
        argv[1] = "1080";

    if (argv[2] == NULL)
        argv[2] = "4";

    if ((argv[2][0] != '4') & (argv[2][0] != '5'))
    {
        LogErr("ConnectTCPSocksProxy: Unsupported socks version level.\n");
        return (DWORD)-1;
    }

    if (argv[3] == NULL)
    {
        LogErr("ConnectTCPSocksProxy: Need a destination host and port "
            "as argument.\n");
        return (DWORD)-1;
    }

    if (strlen(argv[3]) > 254)
    {
        LogErr("ConnectTCPSocksProxy: Too long destination address.\n");
        return (DWORD)-1;
    }

    // Get destination address if it is an IP address
    u_long haddr = inet_addr(argv[3]);

    if (argv[4] == NULL)
        argv[4] = "23";

    // Get destination address
    if (haddr == INADDR_NONE)
    {
        hostent *hent = gethostbyname(argv[3]);
        if (!hent)
        {
            LogErr("ConnectTCPSocksProxyWithLookup: gethostbyname() failed "
                "for %s.\n", argv[3]);
            return (DWORD)-1;
        }

        haddr = *(u_long *)hent->h_addr;
    }

    u_short hport = htons((u_short)atol(argv[4]));
    if (hport == 0)
    {
        servent *sent = getservbyname(argv[4], "tcp");
        if (!sent)
        {
            LogErr("ConnectTCPSocksProxyWithLookup: getservbyname() failed "
                "for %s.\n", argv[4]);
            return (DWORD)-1;
        }

        hport = sent->s_port;
    }

    sockaddr_in addrDest;
    addrDest.sin_family = AF_INET;
    addrDest.sin_port = hport;
    addrDest.sin_addr.s_addr = haddr;
    memset(addrDest.sin_zero, 0, sizeof addrDest.sin_zero);

    LogMsg("Connecting to %s:%s...\n", argv[0], argv[1]);
    SOCKET sRemote = ConnectTCP(argv[0], argv[1]);
    if (sRemote == INVALID_SOCKET)
    {
        LogErr("ConnectTCPHttpProxy: Connection to proxy failed.\n");
        return (DWORD)-1;
    }
    LogMsg("Connected to %s:%s.\n", argv[0], argv[1]);

    SetSockOpts(sRemote);

    WHeapMem<CHAR> pPacket(TCP_PACKET_SIZE);
    if (!pPacket)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    WOverlappedIOC ol;
    if (!ol)
    {
        LogWinErr("ConnectTCPSocksProxy: Memory allocation error");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    pPacket[0] = (UCHAR)atoi(argv[2]);

    // If version 5, do some init
    if (pPacket[0] == '\x05')
    {
        pPacket[1] = '\x01';
        pPacket[2] = '\x00';

        LogMsg("ConnectTCPSocksProxy: Connected to %s:%s, version %i...\n",
            argv[0], argv[1], (int)pPacket[0]);

        if (!ol.BufSend((HANDLE)sRemote, (LPSTR)pPacket, 3))
        {
            LogWinErr("ConnectTCPSocksProxy: Error sending to socks server");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (ol.BufRecv((HANDLE)sRemote, (LPSTR)pPacket, 2) != 2)
        {
            LogWinErr("ConnectTCPSocksProxy: Error receiving from socks "
                "server");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (pPacket[0] == '\xFF')
        {
            LogErr("ConnectTCPSocksProxy: The socks server denied "
                "the request method.\n");
            closesocket(sRemote);
            return (DWORD)-1;
        }

        if (pPacket[1] != '\x00')
        {
            LogErr("ConnectTCPSocksProxy: The socks server requires "
                "authentication.\n");
            closesocket(sRemote);
            return (DWORD)-1;
        }
    }

    LogMsg("Connecting proxy to %s:%i.\n",
        inet_ntoa(addrDest.sin_addr), (int)ntohs(addrDest.sin_port));

    // Connect request
    pPacket[1] = '\x01';
    int iRequestPacketLength;
    if (pPacket[0] == '\x04')
    {
        *(u_short*)(pPacket + 2) = addrDest.sin_port;
        *(u_long*)(pPacket + 4) = addrDest.sin_addr.s_addr;
        pPacket[8] = '\x00';

        iRequestPacketLength = 9;
    }
    else
    {
        pPacket[2] = '\x00';
        pPacket[3] = '\x01';
        *(u_long*)(pPacket + 4) = addrDest.sin_addr.s_addr;
        *(u_short*)(pPacket + 8) = addrDest.sin_port;

        iRequestPacketLength = 10;
    }

    LogMsg("ConnectTCPSocksProxy: Ok, sending request for %s:%u...\n",
        argv[3], (UINT)ntohs(hport));

    if (!ol.BufSend((HANDLE)sRemote, (LPSTR)pPacket, iRequestPacketLength))
    {
        LogWinErr("ConnectTCPSocksProxy: Error sending to socks server");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    if (pPacket[0] == '\x04')
        iRequestPacketLength = 8;
    else
        iRequestPacketLength = 10;

    puts("ConnectTCPSocksProxy: Waiting for reply...");

    DWORD iLen =
        ol.BufRecv((HANDLE)sRemote, (LPSTR)pPacket, iRequestPacketLength);
    if ((iLen == SOCKET_ERROR) | (iLen < 7))
    {
        LogWinErr("ConnectTCPSocksProxy: Error receiving from socks server");
        closesocket(sRemote);
        return (DWORD)-1;
    }

    if (!((pPacket[0] == 0) & (pPacket[1] == 90)) &
        !((pPacket[0] == '\x04') & (pPacket[1] == '\x00')) &
        !((pPacket[0] == '\x05') & (pPacket[1] == '\x00')))
    {
        LogErr("ConnectTCPSocksProxy: The socks server failed to connect ("
            "level %i status %i).\n", (int)pPacket[0], (int)pPacket[1]);
        closesocket(sRemote);
        return pPacket[1];
    }

    in_addr iaddr;
    iaddr.s_addr = *(u_long *)(pPacket + 4);
    if (pPacket[0] == '\x00')
        hport = *(u_short *)(pPacket + 2);
    else
        hport = *(u_short *)(pPacket + 8);

    LogMsg("ConnectTCPSocksProxy: Ok, SOCKS interface is %s:%u.\n",
        inet_ntoa(iaddr), (UINT)ntohs(hport));

    return sRemote;
}
#endif

