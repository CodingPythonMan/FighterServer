#pragma once
#include "Player.h"
#include "RingBuffer.h"

struct Session {
	SOCKET Sock;
	int ID;
	Player* _Player;
	RingBuffer SendBuffer;
	RingBuffer RecvBuffer;
	WCHAR IP[16];
	unsigned short Port;

	Session() {
		Sock = 0;
		ID = 0;
		_Player = new Player;
		memset(IP, 0, 16);
		Port = 0;
	}
};
