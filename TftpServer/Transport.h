#ifndef _TRANSPORT_H
#define _TRANSPORT_H

class Transport {
public:
	virtual bool send(char* data, unsigned int len) = 0;
	virtual int recv(char* buff, unsigned int buff_len) = 0;
};
#endif /* _TRANSPORT_H*/
