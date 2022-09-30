#pragma once

#include "define.h"
#pragma	comment(lib, "../lib/Common.lib")

//	IOCP ���� ������ class
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
	HANDLE	m_hndIOCP;			//	IOCP �ڵ�
	SOCKET	m_sockListen;		//	������ ����
	int		m_iClientCount;		//	���ӵ� Ŭ�� ��

	vector<CLIENT>	m_vecClient;		//	Ŭ���̾�Ʈ ���� ���� �����̳�
	vector<thread>	m_vecWorkThread;	//	�۾� ������ �����̳�
	thread			m_thrdAccepter;		//	Accept ������

	BOOL			m_bIsActive;		//	�۾� ���� �÷���
	BOOL			m_bIsAccept;		//	Accept �÷���

	mutex			m_mtxRecv;			//	����ó���� ���ؽ�
	mutex			m_mtxAccept;		//	

	void	initClient();				//	�ִ� ������ ��ŭ Ŭ�� ���� ����
	BOOL	initWorkerThread();			//	Worker Thread
	BOOL	initAccepterThread();		//	Accept Thread
	BOOL	Bind_IOCP_Port(CLIENT* _lpClient);
	void	closeClientSocket(CLIENT* _lpClient, BOOL _bIsForce = FALSE);

	//	������
	BOOL	WorkerThread();
	void	AccepterThread();

	CLIENT* GetEmptyClient();

	//	Recv Send
	BOOL	BindRecv(CLIENT* _lpClient);
	BOOL	SendMsg(CLIENT* _lpClient, char* _lpMsg, int nLen);
};
