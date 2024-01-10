#pragma once
#include "Packet.h"

void mpCreateMyCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP);

void mpMoveStart(Packet* packet, unsigned int );