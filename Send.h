#pragma once
#include "Session.h"
#include "Packet.h"

void SendPacket_SectorOne(int sectorX, int sectorY, Packet* packet, Session* exceptSession);
void SendPacket_Unicast(Session* session, Packet* packet);
void SendPacket_Around(Session* session, Packet* packet, bool me = false);
void SendPacket_Broadcast(Session* session, Packet* packet);

extern int dx[8];
extern int dy[8];