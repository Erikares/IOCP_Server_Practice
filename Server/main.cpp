#include "Server.h"

int main()
{
	cIOCP_Server	iocpSERVER;
	
	//	서버 기초 셋팅
	iocpSERVER.InitServer();
	iocpSERVER.Bind(dSERVER_PORT);
	iocpSERVER.Listen();

	//	서버 시작
	iocpSERVER.onServer();


	while (TRUE)
	{
		char	szInput[64] = "";
		cin >> szInput;

		//	Q/Y 로 종료.
		if (0 == strcmp(szInput, "q"))
		{
			cout << "Quit Server?" << endl;
			cin >> szInput;
			if (0 == strcmp(szInput, "y"))
			{
				iocpSERVER.DestoryThread();
				break;
			}
		}
	}

	return -1;
}