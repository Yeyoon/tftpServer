#ifndef _UDP_TRANSPORT_H
#define _UDP_TRANSPORT_H

#include "Transport.h"
#include <Winsock2.h>

class UdpTransport : public Transport {
public:
	UdpTransport(int udp_port);
	~UdpTransport();
	bool send(char* data, unsigned int len);
	int recv(char* buff, unsigned int buf_len);

private:
	SOCKET _socket;
	SOCKADDR_IN _addrSrv;
	sockaddr _dstAddr;
	int      _dstAddrLen;
	bool    connected;
};
#endif /*_UDP_TRANSPORT_H*/
