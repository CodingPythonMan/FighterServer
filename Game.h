#pragma once
#include "Character.h"
#include "Packet.h"

void MakeCharacter(unsigned int SessionID);

bool PacketProc(Session* session, unsigned char PacketType, Packet* packet);