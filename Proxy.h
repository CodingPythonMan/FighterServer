#pragma once
#include "Packet.h"

void mpCreateMyCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP);
void mpCreateOtherCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP);
void mpDeleteCharacter(Packet* packet, unsigned int SessionID);

void mpMoveStart(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);
void mpMoveStop(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);

void mpAttack001(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);
void mpAttack002(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);
void mpAttack003(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y);
void mpDamage(Packet* packet, unsigned int AttackID, unsigned int DamageID, char Damage);

void mpSync(Packet* packet, unsigned int SessionID, short X, short Y);
void mpEcho(Packet* packet, unsigned int Time);