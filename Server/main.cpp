#include "Server.h"

int main()
{
	cIOCP_Server	iocpSERVER;
	
	//	���� ���� ����
	iocpSERVER.InitServer();
	iocpSERVER.Bind(dSERVER_PORT);
	iocpSERVER.Listen();

	//	���� ����
	iocpSERVER.onServer();


	while (TRUE)
	{
		char	szInput[64] = "";
		cin >> szInput;

		//	Q/Y �� ����.
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