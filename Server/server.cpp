#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetUtil.h"
#include "ChatPacket.h"
#include "PosPacket.h"

#include <winsock2.h>
#include <iostream>
#include <vector>
#include <map>


#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")

using namespace std;

char Buffer[1024] = { 0, };
std::map<int, pair<int, int>> PlayerData;
std::map<int, string> IconData;

//blocking, synchrous, multiplexing(polling)
int main()
{
	cout << "server start" << endl;

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(35000);

	//already use port 이미 포트 사용중
	::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, SOMAXCONN);



	//blocking, synchronous(TimeOut)
	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 500000;

	fd_set ReadSockets;
	fd_set CopyReadSockets;

	FD_ZERO(&ReadSockets);
	FD_SET(ListenSocket, &ReadSockets);

	while (true)
	{
		CopyReadSockets = ReadSockets;

		//0.5초씩 blocking
		int ChangeCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);

		if (ChangeCount <= 0)
		{
			//Server Work
			//0.5초한번 서버 작업을 하는거
			continue;
		}

		//몬가 자료 있다.
		for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
		{
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					//connect process
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockSockLength = sizeof(ClientSockAddr);

					//blocking, synchronous
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockSockLength);

					cout << "connect client " << inet_ntoa(ClientSockAddr.sin_addr) << endl;

					FD_SET(ClientSocket, &ReadSockets);
				}
				else
				{
					//Data Receive

					//header
					Header RecvHeader;

					int RecvBytes = recv(ReadSockets.fd_array[i], (char*)&RecvHeader, sizeof(RecvHeader), MSG_WAITALL);

					if (RecvBytes <= 0)
					{
						cout << "header recv fail " << endl;
						SOCKADDR_IN ClosedSockAddr;
						memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
						int ClosedSockAddrLength = sizeof(ClosedSockAddr);

						SOCKET ClosedSocket = ReadSockets.fd_array[i];
						getpeername(ClosedSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
						cout << "disconnect client " << inet_ntoa(ClosedSockAddr.sin_addr) << endl;
						FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
						closesocket(ClosedSocket);
						continue;
					}

					RecvHeader.PacketSize = ntohs(RecvHeader.PacketSize);
					RecvHeader.PacketAmount = ntohs(RecvHeader.PacketAmount);

					memset(Buffer, 0, sizeof(Buffer));

					//data JSON
					RecvBytes = recv(ReadSockets.fd_array[i], Buffer, (int)RecvHeader.PacketSize, MSG_WAITALL);
					if (RecvBytes <= 0)
					{
						cout << "data recv fail " << endl;
						SOCKADDR_IN ClosedSockAddr;
						memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
						int ClosedSockAddrLength = sizeof(ClosedSockAddr);

						SOCKET ClosedSocket = ReadSockets.fd_array[i];
						getpeername(ClosedSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
						cout << "disconnect client " << inet_ntoa(ClosedSockAddr.sin_addr) << endl;
						FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
						closesocket(ClosedSocket);
						continue;
					}
					else
					{
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockAddrLength = sizeof(ClientSockAddr);
						//ip 알아내기
						int PeerName = getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

						//이동 연산하기

						ChatPacket Data;
						Data.Parse(Buffer);

						//플레이어 데이터가 없다면 추가하기
						PlayerData.try_emplace(PeerName, pair<int, int>(0, 0));
						IconData.try_emplace(PeerName, Data.Icon);
						//플레이어 기존 위치
						int X = PlayerData[PeerName].first;
						int Y = PlayerData[PeerName].second;

						//위치 이동
						if (Data.MoveCode == "w" && Y > 0)
						{
							Y--;
						}
						if (Data.MoveCode == "s")
						{
							Y++;
						}
						if (Data.MoveCode == "d")
						{
							X++;
						}
						if (Data.MoveCode == "a" && X > 0)
						{
							X--;
						}
						//데이터 저장
						IconData[PeerName] = Data.Icon;
						PlayerData[PeerName] = pair<int, int>(X, Y);

						//위치 Header 만들기
						Header PosHeader;
						PosHeader.PacketAmount = IconData.size();
						PosHeader.PacketSize = sizeof(PosData);
						PosHeader.PacketAmount = htons(PosHeader.PacketAmount);
						PosHeader.PacketSize = htons(PosHeader.PacketSize);

						//모든 접속한 유저한테 전달
						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							if (ReadSockets.fd_array[j] != ListenSocket)
							{
								//header
								SendAll(ReadSockets.fd_array[j], (char*)&PosHeader, sizeof(PosHeader));
								
								//플레이어 인원만큼 위치 데이터 전송
								for (const auto& IconKvp : IconData)
								{
									//PosPacket에 대입
									PosData SendData;
									SendData.Icon = IconData[IconKvp.first][0];
									SendData.PosX = PlayerData[IconKvp.first].first;
									SendData.PosY = PlayerData[IconKvp.first].second;

									SendData.PosX = htons(SendData.PosX);
									SendData.PosY = htons(SendData.PosY);

									//debug
									cout << "client(" << SendData.Icon;
									cout << ") : " << ntohs(SendData.PosX) << ", " << ntohs(SendData.PosY) << " send" << endl;
						

									//Data
									SendAll(ReadSockets.fd_array[j], (char*)&SendData, ntohs(PosHeader.PacketSize));
									
								}
							}
						}
					}
				}
			}
		}
	}






	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}