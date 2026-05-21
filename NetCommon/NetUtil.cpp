#include "pch.h"

#include "NetUtil.h"

void SendAll(SOCKET ReceiverSocket, const char* Data, int Size)
{
	int TotalSendDataSize = 0;
	int WantSendDataSize = Size;
	int SentBytes = 0;
	int Count = 0;
	do
	{
		SentBytes = send(ReceiverSocket, Data + TotalSendDataSize, WantSendDataSize - TotalSendDataSize, 0);
		TotalSendDataSize += SentBytes;
		//printf("Send %dBytes %d Count\n", SentBytes, ++Count);
	} while (TotalSendDataSize < WantSendDataSize);
}
