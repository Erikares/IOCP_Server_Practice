#pragma once
#define dSERVER_PORT		37751
#define dMAX_WORK_THREAD	4
#define dMAX_USER			1000
#define	dMAX_BUF			128

#pragma comment(lib, "ws2_32")

#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

enum ePacketStatus
{
	eRecvPacket = 0,
	eSendPacket,
};

typedef	struct stOverlappedExpand
{
	WSAOVERLAPPED	m_wsaOverlapped;
	SOCKET			m_sockClient;
	WSABUF			m_wsaBuf;
	char			m_strBuf[dMAX_BUF];
	ePacketStatus	m_eStatus;
}OVERLAPEX;

typedef struct stClient
{
	SOCKET		m_sockClient;
	OVERLAPEX	m_stRecvOverlapEx;
	OVERLAPEX	m_stSendOverlapEx;

	stClient()
	{
		ZeroMemory(&m_stRecvOverlapEx, sizeof(OVERLAPEX));
		ZeroMemory(&m_stSendOverlapEx, sizeof(OVERLAPEX));
		m_sockClient = INVALID_SOCKET;
	}
}CLIENT;