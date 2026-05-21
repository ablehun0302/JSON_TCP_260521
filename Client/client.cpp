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

char SendBuffer[HEADER_SIZE] = { 0, };
char RecvBuffer[1024] = { 0, };

bool IsRecvThreadRunning = true;
bool IsSendThreadRunning = true;

//유저 정보
std::string NickName = "ablehun";
const char PlayerIcon = 'A';
//---------

void GotoXY(int x, int y)
{
	// 1. 콘솔창의 출력 핸들(제어권)을 가져옵니다.
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

	// 2. Windows에서 제공하는 좌표 구조체에 X, Y를 담습니다.
	COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };

	// 3. 커서 위치를 변경합니다.
	SetConsoleCursorPosition(output, pos);
}

unsigned WINAPI RecvThread(void* Argument)
{
	SOCKET ServerSocket = *(SOCKET*)Argument;

	while (IsRecvThreadRunning)
	{
		Header RecvHeader;

		//header
		int RecvBytes = recv(ServerSocket, (char*)&RecvHeader, sizeof(RecvHeader), MSG_WAITALL);
		if (RecvBytes <= 0)
		{
			cout << "recv fail " << endl;
			break;
		}

		RecvHeader.PacketSize = ntohs(RecvHeader.PacketSize);
		RecvHeader.PacketAmount = ntohs(RecvHeader.PacketAmount);

		// 위치 넣기
		system("cls");

		for (int i = 0; i < RecvHeader.PacketAmount; i++)
		{
			PosData RecvData;

			RecvBytes = recv(ServerSocket, (char*)&RecvData, RecvHeader.PacketSize, MSG_WAITALL);
			if (RecvBytes <= 0)
			{
				cout << "recv fail " << endl;
				break;
			}

			RecvData.PosX = ntohs(RecvData.PosX);
			RecvData.PosY = ntohs(RecvData.PosY);

			GotoXY(RecvData.PosX, RecvData.PosY);
			cout << RecvData.Icon;
		}
		cout << endl;

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
		Data.UserID = NickName;
		Data.Icon = PlayerIcon;
		Data.MoveCode = Keycode;

		//json 문자열로 만들어줘
		std::string JSONString = Data.ToString();

		Header PacketHeader = { (int)JSONString.length(), 1 };
		PacketHeader.PacketAmount = htons(PacketHeader.PacketAmount);
		PacketHeader.PacketSize = htons(PacketHeader.PacketSize);
		memcpy(SendBuffer, &PacketHeader, HEADER_SIZE);

		//header -> json 크기만 보내봐라
		SendAll(ServerSocket, (const char*)&SendBuffer, HEADER_SIZE);

		//Data -> json 보내라
		SendAll(ServerSocket, JSONString.c_str(), ntohs(PacketHeader.PacketSize));
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
	ServerSockAddr.sin_addr.s_addr = inet_addr("192.168.0.178");
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