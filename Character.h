#pragma once
#include "Session.h"
#include <list>
#include <map>
#include "Protocol.h"

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
	Character(Session* sessionPtr);
	virtual ~Character();

	Session* GetSessionPtr();
	unsigned char GetDirect();
	short GetX();
	short GetY();
	SectorPos GetSectorPos();
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

Character* FindCharacter(unsigned int SessionID);
SectorPos FindSectorPos(unsigned int SessionID);

extern std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
extern std::list<Character*> gSector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];