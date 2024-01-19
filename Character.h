#pragma once
#include "Session.h"
#include "Protocol.h"
#include <list>
#include <map>

#define IS_MOVE 1
#define DEFAULT_HP 100

struct SectorPos {
	short X;
	short Y;
};

class Character
{
public:
	Character(Session* sessionPtr, unsigned int SessionID);
	virtual ~Character();

	void OnDamage(char Damage);
	bool IsDead();

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