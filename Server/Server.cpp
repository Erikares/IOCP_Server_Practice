#include "Server.h"

cIOCP_Server::cIOCP_Server()
	: m_sockListen(INVALID_SOCKET)
	, m_bIsActive(TRUE)
	, m_bIsAccept(TRUE)
{

}

cIOCP_Server::~cIOCP_Server()
{
	WSACleanup();
}

BOOL cIOCP_Server::InitServer()
{
	WSADATA	wsaData;
	if (0 != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		cout << "Error WSAStartUp Failed. #" << WSAGetLastError() << endl;
		return FALSE;
	}

	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_sockListen)
	{
		cout << "Error Listen Socket #" << WSAGetLastError() << endl;
		return FALSE;
	}

	cout << "Socket Init Success!" << endl;

	return TRUE;
}

BOOL	cIOCP_Server::Bind(WORD _wBindPort)
{
	SOCKADDR_IN	sockaddr_Server;
	sockaddr_Server.sin_family = AF_INET;
	sockaddr_Server.sin_port = htons(_wBindPort);
	sockaddr_Server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (0 != bind(m_sockListen, (SOCKADDR*)&sockaddr_Server, sizeof(SOCKADDR_IN)))
	{
		cout << "Error Bind Failed. #" << WSAGetLastError() << endl;
		return FALSE;
	}

	cout << "Bind Success!" << endl;

	return TRUE;
}

BOOL	cIOCP_Server::Listen()
{
	if (0 != listen(m_sockListen, 5))
	{
		cout << "Error Listen Failed. #" << WSAGetLastError() << endl;
		return FALSE;
	}

	cout << "Listening Socket Init Success!" << endl;
	return TRUE;
}

BOOL	cIOCP_Server::onServer()
{
	initClient();

	m_hndIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, dMAX_WORK_THREAD);
	if (NULL == m_hndIOCP)
	{
		cout << "Error CreateIoCompletionPort() Failed. #" << GetLastError() << endl;
		return FALSE;
	}

	BOOL	bIsRet = initWorkerThread();
	if (FALSE == bIsRet)
		return FALSE;

	bIsRet = initAccepterThread();
	if (FALSE == bIsRet)
		return FALSE;

	cout << "OnServer!!" << endl;

	return TRUE;
}

void	cIOCP_Server::initClient()
{
	for (DWORD i = 0; i < dMAX_USER; i++)
		m_vecClient.emplace_back();
}

BOOL	cIOCP_Server::initWorkerThread()
{
	unsigned int uiThreadId = 0;

	//WaitingThread Queue에 대기상태로 넣을 쓰레드들. 권장 개수 cpu ea * 2 + 1
	for (int i = 0; i < dMAX_WORK_THREAD; i++)
		m_vecWorkThread.emplace_back([this]() {WorkerThread(); });

	cout << "Start Worker Thread!!" << endl;

	return TRUE;
}

BOOL	cIOCP_Server::WorkerThread()
{
	CLIENT*			pClient = NULL;
	BOOL			bIsResult = TRUE;
	DWORD			dwIoSize = 0;
	LPOVERLAPPED	lpOverlapped = NULL;

	while (m_bIsActive)
	{
		bIsResult = GetQueuedCompletionStatus(m_hndIOCP,
			&dwIoSize,
			(PULONG_PTR)&pClient,
			&lpOverlapped,
			INFINITE);

		if (TRUE == bIsResult && 0 == dwIoSize && NULL == lpOverlapped)
		{
			m_bIsActive = FALSE;
			continue;
		}

		if (NULL == lpOverlapped)
			continue;

		if (FALSE == bIsResult || (0 == dwIoSize && TRUE == bIsResult))
		{
			cout << '#' << (int)pClient->m_sockClient << " Disconnect client." << endl;
			closeClientSocket(pClient);
				
			continue;
		}

		OVERLAPEX* pOverlappedEx = (OVERLAPEX*)lpOverlapped;

		if (ePacketStatus::eRecvPacket == pOverlappedEx->m_eStatus)
		{
			pOverlappedEx->m_strBuf[dwIoSize] = NULL;
			cout << "Recv. Byte : " << dwIoSize << ". " << pOverlappedEx->m_strBuf << endl;

			//	받은 내용 클라로 쏴주고 수신상태로 재셋팅
			SendMsg(pClient, pOverlappedEx->m_strBuf, dwIoSize);
			BindRecv(pClient);
		}
	}
	return TRUE;
}

void	cIOCP_Server::AccepterThread()
{
	SOCKADDR_IN	stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (m_bIsAccept)
	{
		CLIENT* pClient = GetEmptyClient();
		if (NULL == pClient)
		{
			cout << "Error Client Full" << endl;
			return;
		}

		//	대기
		pClient->m_sockClient = accept(m_sockListen, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (INVALID_SOCKET == pClient->m_sockClient)
			continue;

		BOOL	bRet = Bind_IOCP_Port(pClient);
		if (FALSE == bRet)
			return;

		bRet = BindRecv(pClient);
		if (FALSE == bRet)
			return;

		char szClientIP[32] = { 0, };
		inet_ntop(AF_INET, &(stClientAddr.sin_addr), szClientIP, 32 - 1);
		cout << "Connect Client IP : " << szClientIP << " SOCKET : " << (int)pClient->m_sockClient << endl;

		++m_iClientCount;
	}
}

BOOL	cIOCP_Server::initAccepterThread()
{
	m_thrdAccepter = thread([this]() {AccepterThread(); });

	cout << "Accepter Thread Start." << endl;

	return TRUE;
}

CLIENT* cIOCP_Server::GetEmptyClient()
{
	for (auto& client : m_vecClient)
	{
		if (INVALID_SOCKET == client.m_sockClient)
		{
			return &client;
		}
	}

	return NULL;
}

BOOL	cIOCP_Server::Bind_IOCP_Port(CLIENT* _lpClient)
{
	auto hIOCP = CreateIoCompletionPort((HANDLE)_lpClient->m_sockClient
		, m_hndIOCP
		, (ULONG_PTR)(_lpClient), 0);

	if (NULL == hIOCP || m_hndIOCP != hIOCP)
	{
		cout << "Error CreateIoCompletionPort Failed." << endl;
		return FALSE;
	}

	return TRUE;
}

void	cIOCP_Server::closeClientSocket(CLIENT* _lpClient, BOOL _bIsForce)
{
	struct linger stLinger = { 0,0 };
	if (TRUE == _bIsForce)
		stLinger.l_onoff = 1;

	shutdown(_lpClient->m_sockClient, SD_BOTH);
	setsockopt(_lpClient->m_sockClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	closesocket(_lpClient->m_sockClient);
	_lpClient->m_sockClient = INVALID_SOCKET;
}

BOOL	cIOCP_Server::BindRecv(CLIENT* _lpClient)
{
	DWORD	dwFlag = 0;
	DWORD	dwRecvNumBytes = 0;

	// Overlapped I/O 셋팅
	_lpClient->m_stRecvOverlapEx.m_wsaBuf.len = dMAX_BUF;
	_lpClient->m_stRecvOverlapEx.m_wsaBuf.buf = _lpClient->m_stRecvOverlapEx.m_strBuf;
	_lpClient->m_stRecvOverlapEx.m_eStatus = ePacketStatus::eRecvPacket;

	int nRet = WSARecv(_lpClient->m_sockClient, &(_lpClient->m_stRecvOverlapEx.m_wsaBuf)
		, 1, &dwRecvNumBytes, &dwFlag
		, (LPWSAOVERLAPPED) & (_lpClient->m_stRecvOverlapEx)
		, NULL);

	//	Error. 소켓 끊어짐
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "Error WSARecv() Failed. #" << WSAGetLastError() << endl;
		return FALSE;
	}

	return TRUE;
}

BOOL	cIOCP_Server::SendMsg(CLIENT* _lpClient, char* _lpMsg, int nLen)
{
	DWORD	dwRecvNumBytes = 0;
	
	//	메세지 복사
	CopyMemory(_lpClient->m_stSendOverlapEx.m_strBuf, _lpMsg, nLen);
	
	//	OverLapped I/O 를 위해 정보 셋팅
	_lpClient->m_stSendOverlapEx.m_wsaBuf.len = nLen;
	_lpClient->m_stSendOverlapEx.m_wsaBuf.buf = _lpClient->m_stSendOverlapEx.m_strBuf;
	_lpClient->m_stSendOverlapEx.m_eStatus = ePacketStatus::eSendPacket;

	int nRet = WSASend(_lpClient->m_sockClient,
		&(_lpClient->m_stSendOverlapEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (_lpClient->m_stSendOverlapEx),
		NULL);

	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() Error!!" << endl;
		return FALSE;
	}
	
	return TRUE;
}

void	cIOCP_Server::DestoryThread()
{
	m_bIsActive = FALSE;
	CloseHandle(m_hndIOCP);
	
	for (auto& th : m_vecWorkThread)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	m_bIsAccept = FALSE;
	closesocket(m_sockListen);

	if (m_thrdAccepter.joinable())
	{
		m_thrdAccepter.join();
	}
}