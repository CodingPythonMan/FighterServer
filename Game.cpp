#include "Game.h"
#include "Sector.h"
#include "Log.h"
#include "Packet.h"
#include "Proxy.h"
#include "Send.h"

void Update()
{
	short dx;
	short dy;
	// 다른 CPP 쪽에서 플레이어 리스트를 들고 와야하고,
	// 해당 플레이어 정보에 따라 움직이게 해야한다.
	for (auto iter = gCharacterMap.begin(); iter != gCharacterMap.end(); ++iter)
	{
		Character* character = (*iter).second;

		if (character->Action >= dfPACKET_MOVE_STOP)
			continue;
		
		switch (character->MoveDirect)
		{
		case dfPACKET_MOVE_DIR_LL:
			dx = -dfSPEED_PLAYER_X;
			dy = 0;
			break;
		case dfPACKET_MOVE_DIR_LU:
			dx = -dfSPEED_PLAYER_X;
			dy = -dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_UU:
			dx = 0;
			dy = -dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_RU:
			dx = dfSPEED_PLAYER_X;
			dy = -dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_RR:
			dx = dfSPEED_PLAYER_X;
			dy = 0;
			break;
		case dfPACKET_MOVE_DIR_RD:
			dx = dfSPEED_PLAYER_X;
			dy = dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_DD:
			dx = 0;
			dy = dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_LD:
			dx = -dfSPEED_PLAYER_X;
			dy = dfSPEED_PLAYER_Y;
			break;
		default:
			_LOG(LOG_LEVEL_ERROR, L"GameLogic => Direction Error!");
			return;
		}

		if ((character->X + dx >= dfRANGE_MOVE_LEFT && character->X + dx < dfRANGE_MOVE_RIGHT)
			&& (character->Y + dy >= dfRANGE_MOVE_TOP && character->Y + dy < dfRANGE_MOVE_BOTTOM))
		{
			character->X += dx;
			character->Y += dy;
		}

		if (SectorUpdateCharacter(character))
		{
			SectorUpdatePacket(character);
			SendMoveStartNewSection(character);
		}
	}
}

void SendMoveStartNewSection(Character* character)
{
	SectorPos sector = character->Sector;

	Packet MoveStart;
	mpMoveStart(&MoveStart, character->SessionID, character->MoveDirect,
		character->X, character->Y);
	switch (character->Action)
	{
	case dfPACKET_MOVE_DIR_LL:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_LU:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_UU:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_RU:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_RR:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_RD:
		SendPacket_SectorOne(sector.X + 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_DD:
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		break;
	case dfPACKET_MOVE_DIR_LD:
		SendPacket_SectorOne(sector.X - 1, sector.Y - 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X - 1, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X, sector.Y + 1, &MoveStart, nullptr);
		SendPacket_SectorOne(sector.X + 1, sector.Y + 1, &MoveStart, nullptr);
		break;
	default:
		_LOG(LOG_LEVEL_ERROR, L"GameLogic => Direction Error!");
		return;
	}
}