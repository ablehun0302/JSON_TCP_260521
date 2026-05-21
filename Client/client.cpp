#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include "ChatPacket.h"
#include "PosPacket.h"
#include "NetUtil.h"

#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <process.h>
#include <conio.h>




#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")


using namespace std;

char SendBuffer[1024] = { 0, };
char RecvBuffer[1024] = { 0, };

bool IsRecvThreadRunning = true;
bool IsSendThreadRunning = true;

unsigned WINAPI RecvThread(void* Argument)
{
	SOCKET ServerSocket = *(SOCKET*)Argument;

	while (IsRecvThreadRunning)
	{
		unsigned short PacketSize = 0;

		//header
		int RecvBytes = recv(ServerSocket, (char*)&PacketSize, sizeof(PacketSize), MSG_WAITALL);
		if (RecvBytes <= 0)
		{
			cout << "recv fail " << endl;
			break;
		}

		PacketSize = ntohs(PacketSize);

		memset(RecvBuffer, 0, sizeof(RecvBuffer));
		//data JSON
		RecvBytes = recv(ServerSocket, RecvBuffer, PacketSize, MSG_WAITALL);
		if (RecvBytes <= 0)
		{
			cout << "recv fail " << endl;
			break;
		}

		PosPacket Data;

		Data.Parse(RecvBuffer);

		cout << Data.UserID << " : (" << Data.PosX << ", " << Data.PosY << ")" << endl;
	}


	return 0;
}

unsigned WINAPI SendThread(void* Argument)
{
	//책임은 사용하는 놈이 진다.
	SOCKET ServerSocket = *(SOCKET*)Argument;

	while (IsSendThreadRunning)
	{
		char Keycode = _getch();

		//json 데이터 만들거야
		ChatPacket Data;
		Data.UserID = "ablehun";
		Data.MoveCode = Keycode;

		//json 문자열로 만들어줘
		std::string JSONString = Data.ToString();

		unsigned short PacketSize = (unsigned short)JSONString.length();
		PacketSize = htons(PacketSize);

		//header -> json 크기만 보내봐라
		SendAll(ServerSocket, (char*)&PacketSize, 2);

		//Data -> json 보내라
		SendAll(ServerSocket, JSONString.c_str(), ntohs(PacketSize));
	}

	return 0;
}

int main()
{
	cout << "client" << endl;


	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(35000);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	cout << "client connect" << endl;

	HANDLE ThreadHandles[2] = { 0, };

	//nonblocking, asynchrous
	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, RecvThread, &ServerSocket, /*CREATE_SUSPENDED*/0, 0);
	ThreadHandles[1] = (HANDLE)_beginthreadex(0, 0, SendThread, &ServerSocket, /*CREATE_SUSPENDED*/0, 0);
	//ResumeThread(ThreadHandles[0]);
	//ResumeThread(ThreadHandles[1]);
	//SuspendThread(ThreadHandles[0]);
	//SuspendThread(ThreadHandles[1]);


	//blocking
	WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

	closesocket(ServerSocket);

	//TerminateThread(ThreadHandles[0], 0);
	//TerminateThread(ThreadHandles[1], 0);
	IsSendThreadRunning = false;
	IsRecvThreadRunning = false;


	CloseHandle(ThreadHandles[0]);
	CloseHandle(ThreadHandles[1]);

	WSACleanup();

	return 0;
}