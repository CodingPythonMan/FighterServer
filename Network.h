#pragma once
#include "Session.h"
#include "Packet.h"
#include "Character.h"

#define SERVER_PORT 20000

class Network
{
public:
	Network();
	virtual ~Network();

	bool StartUp();
	void IOProcess();

private:
	void SelectSocket(SOCKET* socketSet,int sockCount, FD_SET* rsetPtr, FD_SET* wsetPtr);

	void AcceptProc();
	void ReadProc(SOCKET sock);
	bool WriteProc(SOCKET sock);

	bool PacketProc(Session* session, unsigned char packetType, Packet* packet);

	Session* FindSession(SOCKET socket);
	Session* CreateSession(SOCKET socket);
	void DisconnectSession(Session* session);
	void DeleteSessions();

private:
	SOCKET _listenSock;
	std::map<SOCKET, Session*> _sessionMap;

	std::list<Session*> _deleteList;

	unsigned int _uniqueID;
};