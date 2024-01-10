#include "Character.h"

// ĳ���� ����
std::map<unsigned int, Character*> gCharacterMap;

// ����� ĳ���� ����
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

	// Test ������ ���� �ڵ�
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

	// ĳ���� �߰�
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