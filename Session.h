#pragma once

#include <WS2tcpip.h>
#include <windows.h>
#include "RingBuffer.h"

#pragma comment(lib, "ws2_32.lib")

struct Session {
	SOCKET Socket;
	unsigned int SessionID;
	RingBuffer RecvQ;
	RingBuffer SendQ;
	unsigned int LastRecvTime;

	Session(SOCKET socket, unsigned int sessionID)
	{
		Socket = socket;
		SessionID = sessionID;
		LastRecvTime = timeGetTime();
	}
};