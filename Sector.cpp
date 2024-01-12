#include "Sector.h"
#include "Send.h"
#include "Proxy.h"

bool SectorUpdateCharacter(Character* character)
{
	// 기존 섹터 기억
	SectorPos beforeSector = character->Sector;

	short newX = character->X / dfSECTOR_X;
	short newY = character->Y / dfSECTOR_Y;

	if (beforeSector.X == newX && beforeSector.Y == newY)
		return false;

	character->Sector.X = newX;
	character->Sector.Y = newY;

	short differX = newX - beforeSector.X;
	short differY = newY - beforeSector.Y;

	if (differX == 1 && differY == 0)
	{
		character->Action = dfPACKET_MOVE_DIR_RR;
	}
	else if (differX == 1 && differY == -1)
	{
		character->Action = dfPACKET_MOVE_DIR_RU;
	}
	else if (differX == 0 && differY == -1)
	{
		character->Action = dfPACKET_MOVE_DIR_UU;
	}
	else if (differX == -1 && differY == -1)
	{
		character->Action = dfPACKET_MOVE_DIR_LU;
	}
	else if (differX == -1 && differY == 0)
	{
		character->Action = dfPACKET_MOVE_DIR_LL;
	}
	else if (differX == -1 && differY == 1)
	{
		character->Action = dfPACKET_MOVE_DIR_LD;
	}
	else if (differX == 0 && differY == 1)
	{
		character->Action = dfPACKET_MOVE_DIR_DD;
	}
	else if (differX == 1 && differY == 1)
	{
		character->Action = dfPACKET_MOVE_DIR_RD;
	}
	
	// 기존 CharacterList 에서 제거
	gSector[beforeSector.Y][beforeSector.X].remove(character);

	// 캐릭터 추가
	gSector[newY][newX].push_back(character);

	return true;
}

void SectorUpdatePacket(Character* character)
{
	SectorPos sector = character->Sector;

	Packet Add;
	mpCreateOtherCharacter(&Add, character->SessionID, character->Direct, 
		character->X, character->Y, character->HP);
	Packet Sub;
	mpDeleteCharacter(&Sub, character->SessionID);

	bool moveStart = false;
	if (character->Action < dfPACKET_MOVE_STOP)
		moveStart = true;

	switch (character->Action)
	{
	case dfPACKET_MOVE_DIR_LL:
		// 1. 생기는 섹터에 대해 정보 뿌리기
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		}

		// 2. 없어지는 섹터에 대한 캐릭터 삭제
		SendPacket_SectorOne(sector.X + 2, sector.Y - 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 2, sector.Y, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 2, sector.Y + 1, &Sub, nullptr);
		
		// 3. 생기는 섹터에 대해 정보 받기
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y + 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y + 1, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_LU:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X + 2, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y - 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_UU:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X - 1, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 2, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y - 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X - 1, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y + 2, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_RU:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X - 2, sector.Y, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y + 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 2, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y - 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 1, sector.Y + 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y + 2, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_RR:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X - 2, sector.Y - 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y + 1, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y + 1, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_RD:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X - 2, sector.Y, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y - 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 2, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 2, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y + 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 2, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X - 1, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y - 2, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_DD:
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X - 1, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 2, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X - 1, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y - 2, &Add, character->SessionPtr);
		break;
	case dfPACKET_MOVE_DIR_LD:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &Add, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &Add, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &Add, nullptr);
		if (moveStart == true)
		{
			Packet MoveStart;
			mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
			SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		}

		SendPacket_SectorOne(sector.X + 2, sector.Y, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 2, sector.Y - 1, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 2, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 2, &Sub, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 2, &Sub, nullptr);

		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y - 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X - 1, sector.Y + 1, &Add, character->SessionPtr); 
		ReceivePacket_CreateSectorOne(sector.X, sector.Y + 1, &Add, character->SessionPtr);
		ReceivePacket_CreateSectorOne(sector.X + 1, sector.Y + 1, &Add, character->SessionPtr);

		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y -  1, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 2, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X + 1, sector.Y - 2, &Add, character->SessionPtr);
		ReceivePacket_DeleteSectorOne(sector.X, sector.Y - 2, &Add, character->SessionPtr);
		break;
	}
}

void ReceivePacket_CreateSectorOne(int sectorX, int sectorY, Packet* packet, Session* session)
{
	std::list<Character*> characterList = gSector[sectorY][sectorX];
	for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
	{
		Character* character = *iter;
		mpCreateOtherCharacter(packet, character->SessionID, character->Direct,
			character->X, character->Y, character->HP);

		SendPacket_Unicast(session, packet);

		if (character->Action < dfPACKET_MOVE_STOP)
		{
			mpMoveStart(packet, character->SessionID, character->MoveDirect,
				character->X, character->Y);

			SendPacket_Unicast(session, packet);
		}
	}
}

void ReceivePacket_DeleteSectorOne(int sectorX, int sectorY, Packet* packet, Session* session)
{
	std::list<Character*> characterList = gSector[sectorY][sectorX];
	for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
	{
		mpDeleteCharacter(packet, (*iter)->SessionID);
		SendPacket_Unicast(session, packet);
	}
}