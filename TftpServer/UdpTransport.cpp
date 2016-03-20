#include "UdpTransport.h"
#include <iostream>
#include <Winsock2.h>  

using namespace std;

UdpTransport::UdpTransport(int udp_port)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err)
	{
		cout << "Socket get error" << endl;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		cout << "Socket get error" << endl;
	}

	_socket = socket(AF_INET, SOCK_DGRAM, 0);
	_addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_addrSrv.sin_family = AF_INET;
	_addrSrv.sin_port = htons(udp_port);

	bind(_socket, (SOCKADDR*)&_addrSrv, sizeof(SOCKADDR));
	connected = false;
}

UdpTransport::~UdpTransport()
{
	closesocket(_socket);
}

bool UdpTransport::send(char* data, unsigned int len)
{
	return len == sendto(_socket, data, len, 0, (const sockaddr*)&_dstAddr, sizeof(_dstAddr));
}


int UdpTransport::recv(char *buff, unsigned int buf_len)
{
	sockaddr addr;
	int      addrLen;
	int		 recvLen;

	recvLen = recvfrom(_socket,buff, buf_len, 0, &addr, &addrLen);

	if (!connected)
	{
		_dstAddr = addr;
	}
	else
	{
		if (memcmp(&addr, &_dstAddr, addrLen))
		{
			return -1;
		}
	}

	return recvLen;
}