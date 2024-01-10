#include "Character.h"
#include "GameVariable.h"

Character::Character() : Sector()
{
	SessionPtr = nullptr;
	SessionID = 0;
	Action = DONT_MOVE;
	Direct = dfPACKET_DIR_L;
	MoveDirect = 0;
	X = rand() % dfRANGE_MOVE_RIGHT;
	Y = rand() % dfRANGE_MOVE_BOTTOM;
	HP = DEFAULT_HP;
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