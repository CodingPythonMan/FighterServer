#include "Character.h"

// 캐릭터 관리
std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
std::list<Character*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

int Temp = 0;

Character::Character()
{
	SessionPtr = nullptr;
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

	if (Temp == 1)
	{
		X = 60;
		Y = 60;
		Temp++;
	}

	Sector.X = X / dfSECTOR_MAX_X;
	Sector.Y = Y / dfSECTOR_MAX_Y;
	HP = DEFAULT_HP;

	// 캐릭터 추가
	gSector[Sector.Y][Sector.X].push_back(this);
}

Character::~Character()
{
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

char Character::GetHP()
{
	return HP;
}

SectorPos* Character::GetSectorPtr()
{
	return &Sector;
}