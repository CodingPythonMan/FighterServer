#include "Game.h"
#include <map>
#include <list>
#include "Protocol.h"

// 캐릭터 관리
std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
std::list<Character*> gSector[30][30];

void MakeCharacter(unsigned int SessionID)
{
	Character* character = new Character;
	gCharacterMap.insert({SessionID, character});

	// 1. 당사자에게 생성됐음을 알려줘야함.
	
}

void SendPacket_Unicast(Session* session, Packet* packet)
{

}

void SendPacket_Around(Session* session, Packet* packet, bool me, int sectors)
{

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

bool Proc_MoveStart(Session* session, Packet* packet)
{

	return false;
}