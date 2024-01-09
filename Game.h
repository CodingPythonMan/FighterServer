#pragma once
#include "Character.h"
#include "Packet.h"

#define HEADER_SIZE 3

void MakeCharacter(unsigned int SessionID);

void SendPacket_Unicast(Session* session, Packet* packet);
void SendPacket_Around(Session* session, Packet* packet, bool me = false, int sectors = 9);

bool PacketProc(Session* session, unsigned char PacketType, Packet* packet);

bool Proc_MoveStart(Session* session, Packet* packet);