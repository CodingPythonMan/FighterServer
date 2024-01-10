#pragma once
#include "Session.h"

#define DONT_MOVE 0
#define IS_MOVE 1
#define DEFAULT_HP 100

struct SectorPos {
	short X;
	short Y;
};

class Character
{
public:
	Character();
	virtual ~Character();

	unsigned char GetDirect();
	short GetX();
	short GetY();
	char GetHP();

private:
	Session* SessionPtr;
	unsigned int SessionID;
	unsigned int Action;
	unsigned char Direct;
	unsigned char MoveDirect;
	short X;
	short Y;
	SectorPos Sector;
	char HP;
};