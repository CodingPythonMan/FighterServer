#include "Game.h"
#include <map>
#include <list>
#include "Protocol.h"
#include "Proxy.h"

// ĳ���� ����
std::map<unsigned int, Character*> gCharacterMap;

// ����� ĳ���� ����
std::list<Character*> gSector[30][30];

void MakeCharacter(unsigned int SessionID)
{
	Character* character = new Character;
	gCharacterMap.insert({SessionID, character});

	// 1. ����ڿ��� ���������� �˷������.
	
}

bool PacketProc(Session* session, unsigned char PacketType, Packet* packet)
{
	switch (PacketType)
	{
	case dfPACKET_CS_MOVE_START:
		return Proc_MoveStart(session, packet);
		break;

	}

	return true;
}