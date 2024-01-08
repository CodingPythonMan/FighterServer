#pragma once
#include "Session.h"
#include <map>

#define SERVER_PORT 20000

class Network
{
public:
	bool StartUp();
	void IOProcess();

private:
	void SelectSocket(SOCKET* socketSet, FD_SET* rsetPtr, FD_SET* wsetPtr);

	void AcceptProc();
	void ReadProc(SOCKET sock);
	void WriteProc(SOCKET sock);

private:
	SOCKET _listenSock;
	std::map<SOCKET, Session*> _sessionMap;
};