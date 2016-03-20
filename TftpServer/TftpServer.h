#ifndef _TFTPSERVER_H
#define _TFTPSERVER_H

#include "UdpTransport.h"

typedef enum {
	TFTPOPCODE_RRQ = 1,
	TFTPOPCODE_WRQ,
	TFTPOPCODE_DATA,
	TFTPOPCODE_ACK,
	TFTPOPCODE_ERR,
	TFTPOPCODE_CNT,
}tftpOpcode_e;

typedef enum {
	TFTPSERV_START,
	TFTPSERV_STOP,
}tftpServ_status;

typedef struct RRQ_WRQ {
	char* filename;
	char* mode;
}RRQ,WRQ;

typedef struct DATA {
	unsigned short block;
	char* data;
}DATA;

typedef struct ACK {
	unsigned short block;
}ACK;

typedef struct _ERROR {
	unsigned short errorCode;
	char* errMsg;
}_ERROR;

typedef struct tftpPacket {
	unsigned short opCode;
	union content 
	{
		RRQ		rrq;
		WRQ     wrq;
		DATA	data;
		ACK		ack;
		_ERROR   error;
	}packetContent;
}tftpPacket;

typedef enum {
	TFTPERR_NOT_DEFINED,
	TFTPERR_FILE_NOT_FOUND,
	TFTPERR_ACCESS_VIOLATION,
	TFTPERR_ALLOC_ERROR,
	TFTPERR_UNKNOWN_TRANSFER_ID,
	TFTPERR_FILE_ALREADY_EXISTS,
	TFTPERR_NO_SUCH_USER,
}tftpErrorCode_e;

#define TFTP_DATA_LEN 512
#define TFTP_BUFF_LEN 1024

class TftpServ {
public:
	TftpServ(unsigned short port);
	void start();
	void stop();
private:
	void handlePacket(tftpPacket* packet, int packetLen);
	bool handleRrq(RRQ* rrq);
	bool handleWrq(WRQ* wrq);
	bool handleData(DATA* data, int dataLen);
	bool handleAck(ACK* ack);
	bool handleError(_ERROR* err);

	int sendBlockData(int len);
	int sendError(tftpErrorCode_e errCode,char* errMsg);
	int fillBufffromFile();
	int sendAck();

	UdpTransport *m_pTransport;
	sockaddr		m_Addr;
	unsigned short	m_udpPort;
	unsigned short  peer_udpPort;
	unsigned short  m_blockNumber;

	FILE* m_file;
	tftpServ_status m_tftpServStatus;

	char m_buff[TFTP_BUFF_LEN];
	int m_buff_len;
};
#endif
