#include "Character.h"

// 캐릭터 관리
std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
std::list<Character*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

int Temp = 0;

Character::Character(Session* sessionPtr)
{
	SessionPtr = sessionPtr;
	SessionID = 0;
	Action = DONT_MOVE;
	Direct = dfPACKET_DIR_L;
	MoveDirect = 0;
	X = rand() % dfRANGE_MOVE_RIGHT;
	Y = rand() % dfRANGE_MOVE_BOTTOM;

	// Test 끝나면 지울 코드
	if (Temp == 0)
	{
		X = 30;
		Y = 30;
		Temp++;
	}
	else if (Temp == 1)
	{
		X = 60;
		Y = 60;
		Temp++;
	}

	Sector.X = X / dfSECTOR_X;
	Sector.Y = Y / dfSECTOR_Y;
	HP = DEFAULT_HP;

	// 캐릭터 추가
	gSector[Sector.Y][Sector.X].push_back(this);
}

Character::~Character()
{
}

Session* Character::GetSessionPtr()
{
	return SessionPtr;
}

unsigned char Character::GetDirect()
{
	return Direct;
}

short Character::GetX()
{
	return X;
}

short Character::GetY()
{
	return Y;
}

SectorPos Character::GetSectorPos()
{
	return Sector;
}

char Character::GetHP()
{
	return HP;
}

Character* FindCharacter(unsigned int SessionID)
{
	return gCharacterMap[SessionID];
}

SectorPos FindSectorPos(unsigned int SessionID)
{
	Character* character = FindCharacter(SessionID);
	return character->GetSectorPos();
}