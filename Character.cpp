#include "Character.h"

// 캐릭터 관리
std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
std::list<Character*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

int Temp = 0;

Character::Character(Session* sessionPtr, unsigned int sessionID)
{
	SessionPtr = sessionPtr;
	SessionID = sessionID;
	Action = dfPACKET_MOVE_STOP;
	Direct = dfPACKET_MOVE_DIR_LL;
	MoveDirect = 0;
	X = rand() % dfRANGE_MOVE_RIGHT;
	Y = rand() % dfRANGE_MOVE_BOTTOM;

	// Test 끝나면 지울 코드
	if (Temp == 0)
	{
		X = 205;
		Y = 205;
		Temp++;
	}
	else if (Temp == 1)
	{
		X = 255;
		Y = 255;
		Temp++;
	}
	else if (Temp == 2)
	{
		X = 300;
		Y = 300;
		Temp++;
	}
	else if (Temp == 3)
	{
		X = 280;
		Y = 280;
		Temp++;
	}

	Sector.X = X / dfSECTOR_X;
	Sector.Y = Y / dfSECTOR_Y;
	HP = DEFAULT_HP;

	// 캐릭터 추가
	gSector[Sector.Y][Sector.X].push_back(this);
	gCharacterMap.insert({ SessionID, this});
}

Character::~Character()
{
	gSector[Sector.Y][Sector.X].remove(this);
	gCharacterMap.erase(SessionID);
}

Character* FindCharacter(unsigned int SessionID)
{
	return gCharacterMap[SessionID];
}

SectorPos FindSectorPos(unsigned int SessionID)
{
	Character* character = FindCharacter(SessionID);
	return character->Sector;
}