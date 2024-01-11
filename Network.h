#pragma once
#include "Session.h"
#include "Packet.h"
#include <map>

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
	void WriteProc(SOCKET sock);

	bool PacketProc(Session* session, unsigned char packetType, Packet* packet);

	void SendPacket_SectorOne(int sectorX, int sectorY, Packet* packet, Session* exceptSession);
	void SendPacket_Unicast(Session* session, Packet* packet);
	void SendPacket_Around(Session* session, Packet* packet, bool me = false);
	void SendPacket_Broadcast(Session* session, Packet* packet);

	Session* FindSession(SOCKET socket);

private:
	SOCKET _listenSock;
	std::map<SOCKET, Session*> _sessionMap;

	unsigned int _uniqueID;
};