#pragma once
#include "Packet.h"

void mpCreateMyCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP);
void mpCreateOtherCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP);
void mpDeleteCharacter(Packet* packet, unsigned int SessionID);

void mpMoveStart(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);
void mpSync(Packet* packet, unsigned int SessionID, short X, short Y);