#pragma once
#include "Session.h"
#include "Packet.h"
#include <list>

void SendPacket_SectorOne(int sectorX, int sectorY, Packet* packet, Session* exceptSession);
void SendPacket_Unicast(Session* session, Packet* packet);
void SendPacket_Around(Session* session, Packet* packet, bool me = false);

void DisconnectSession(Session* session);

extern int dx[8];
extern int dy[8];
extern std::list<Session*> _deleteList;