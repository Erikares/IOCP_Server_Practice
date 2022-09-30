#pragma once

#include "define.h"
#pragma	comment(lib, "../lib/Common.lib")

//	IOCP 서버 구현용 class
class cIOCP_Server
{
public:
	cIOCP_Server();
	~cIOCP_Server();

	BOOL	InitServer();
	BOOL	Bind(WORD _wBindPort);
	BOOL	Listen();

	BOOL	onServer();
	void	DestoryThread();


private:
	HANDLE	m_hndIOCP;			//	IOCP 핸들
	SOCKET	m_sockListen;		//	리슨용 소켓
	int		m_iClientCount;		//	접속된 클라 수

	vector<CLIENT>	m_vecClient;		//	클라이언트 정보 보관 컨테이너
	vector<thread>	m_vecWorkThread;	//	작업 스레드 컨테이너
	thread			m_thrdAccepter;		//	Accept 스레드

	BOOL			m_bIsActive;		//	작업 동작 플래그
	BOOL			m_bIsAccept;		//	Accept 플래그

	mutex			m_mtxRecv;			//	수신처리용 뮤텍스
	mutex			m_mtxAccept;		//	

	void	initClient();				//	최대 동접수 만큼 클라 공간 셋팅
	BOOL	initWorkerThread();			//	Worker Thread
	BOOL	initAccepterThread();		//	Accept Thread
	BOOL	Bind_IOCP_Port(CLIENT* _lpClient);
	void	closeClientSocket(CLIENT* _lpClient, BOOL _bIsForce = FALSE);

	//	스레드
	BOOL	WorkerThread();
	void	AccepterThread();

	CLIENT* GetEmptyClient();

	//	Recv Send
	BOOL	BindRecv(CLIENT* _lpClient);
	BOOL	SendMsg(CLIENT* _lpClient, char* _lpMsg, int nLen);
};
