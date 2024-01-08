#include <WS2tcpip.h>
#include <windows.h>
#include "RingBuffer.h"

#pragma comment(lib, "ws2_32.lib")

struct Session {
	SOCKET Socket;
	unsigned int SessionID;
	RingBuffer RecvQ;
	RingBuffer SendQ;

};