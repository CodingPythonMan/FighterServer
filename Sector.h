#pragma once
#include "Character.h"
#include "Packet.h"

bool SectorUpdateCharacter(Character* character);
void SectorUpdatePacket(Character* character);

void ReceivePacket_CreateSectorOne(int sectorX, int sectorY, Packet* packet, Session* session);
void ReceivePacket_DeleteSectorOne(int sectorX, int sectorY, Packet* packet, Session* session);