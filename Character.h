#pragma once
#include "Session.h"
#include "Direction.h"

struct SectorPos {
	short X;
	short Y;
};

struct Character
{
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