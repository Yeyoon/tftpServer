#include "TftpServer.h"

#include <fstream>
#include <iostream>

using namespace std;

TftpServ::TftpServ(unsigned short port)
{
	m_udpPort = port;
}

bool TftpServ::handleAck(ACK* ack)
{
	if (ack->block < m_blockNumber)
	{
		cout << "retransf last data" << endl;
		return m_buff_len == sendBlockData(m_buff_len);
	}
	else if (ack->block == m_blockNumber)
	{
		m_buff_len = fillBufffromFile();
		if (m_buff_len > 0)
		{
			return m_buff_len == sendBlockData(m_buff_len);
		}
		return false;
	}

	return false;
}

bool TftpServ::handleRrq(RRQ* rrq)
{
	char* filename = rrq->filename;
	if (m_file)
	{
		sendError(TFTPERR_NOT_DEFINED,"duplicate read request");
		return false;
	}

	m_file = fopen(filename, "r");
	if (!m_file)
	{
		sendError(TFTPERR_ACCESS_VIOLATION,"file open error");
		return false;
	}

	m_buff_len = fillBufffromFile();

	if (m_buff_len > 0)
	{
		return m_buff_len == sendBlockData(m_buff_len);
	}

	return false;
}

int TftpServ::fillBufffromFile()
{
	if (m_file)
	{
		return fread(m_buff, TFTP_DATA_LEN, 1, m_file);
	}

	return -1;
}

int TftpServ::sendBlockData(int len)
{
	m_blockNumber++;
	tftpPacket* packet;
	memmove(m_buff + 4, m_buff, len);
	packet = (tftpPacket*)m_buff;
	packet->opCode = TFTPOPCODE_DATA;
	packet->packetContent.data.block = m_blockNumber;

	if (m_pTransport->send(m_buff, len+4))	  // total len is datalen + 4
	{
		return len;
	}

	return -1;
}

bool TftpServ::handleWrq(WRQ* wrq)
{
	wrq->mode = wrq->filename + strlen(wrq->filename);
	if (m_file)
	{
		sendError(TFTPERR_FILE_ALREADY_EXISTS,"duplicate write request");
		return false;
	}

	m_file = fopen(wrq->filename, "w");
	if (!m_file)
	{
		sendError(TFTPERR_ACCESS_VIOLATION,"file open for write error");
		return false;
	}

	return sendAck();
}

int TftpServ::sendError(tftpErrorCode_e e, char* errMsg)
{
	tftpPacket* p = (tftpPacket*)m_buff;
	p->opCode = TFTPOPCODE_ERR;
	strncpy(p->packetContent.error.errMsg, errMsg, strlen(errMsg));
	p->packetContent.error.errMsg[strlen(errMsg)] = 0;
	p->packetContent.error.errorCode = e;

	m_pTransport->send(m_buff, strlen(errMsg) + 5);
}

int TftpServ::sendAck()
{
	tftpPacket* packet = (tftpPacket*)m_buff;
	packet->opCode = TFTPOPCODE_ACK;
	packet->packetContent.ack.block = m_blockNumber;
	m_blockNumber++;

	m_pTransport->send(m_buff, 4)?4:-1; // ack len is 4
}

bool TftpServ::handleData(DATA* data, int dataLen)
{
	// this only happens in wrq
	if (m_file && data->block == m_blockNumber + 1)
	{
		fwrite(data->data, dataLen, 1, m_file);
		m_blockNumber++;

		if (dataLen < TFTP_DATA_LEN)
		{
			// this is the terminal packet
			sendAck();
			stop();
		}
		return true;
	}

	return false;
}

bool TftpServ::handleError(_ERROR* err)
{
	cout << "errCode : " << err->errorCode << "\terrMsg : " << err->errMsg << endl;
	stop();
	return false;
}

void TftpServ::handlePacket(tftpPacket* packet, int packetLen)
{
	tftpOpcode_e op = (tftpOpcode_e)packet->opCode;
	bool ret = true;
	switch (op)
	{
	case TFTPOPCODE_ACK:
		ret = handleAck(&(packet->packetContent.ack));
		break;

	case TFTPOPCODE_DATA:
		int dataLen = packetLen - sizeof(unsigned short) - sizeof(unsigned short); // opcode len + blockNumber len
		ret = handleData(&(packet->packetContent.data), dataLen);
		break;

	case TFTPOPCODE_RRQ:
		ret = handleRrq(&(packet->packetContent.rrq));
		break;

	case TFTPOPCODE_WRQ:
		ret = handleWrq(&(packet->packetContent.wrq));
		break;

	case TFTPOPCODE_ERR:
		ret = handleError(&(packet->packetContent.error));
		break;

	default:
		cout << "WRONG TFTP Operation : " << op << endl;
		break;
	}

	if (!ret)
	{
		stop();
	}
}

void TftpServ::start()
{
	m_pTransport = new UdpTransport(m_udpPort);
	m_tftpServStatus = TFTPSERV_START;
	
	while (m_tftpServStatus == TFTPSERV_START)
	{
		int recvLen = m_pTransport->recv(m_buff, TFTP_BUFF_LEN);
		if (recvLen <= 0)
		{
			// something wrong just ignore this time
			continue;
		}

		handlePacket((tftpPacket*)m_buff, recvLen);
	}
}

void TftpServ::stop()
{
	m_tftpServStatus = TFTPSERV_STOP;

	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}
}

