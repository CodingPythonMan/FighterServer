#include "Game.h"
#include <map>
#include <list>

// 캐릭터 관리
std::map<unsigned int, Character*> gCharacterMap;

// 월드맵 캐릭터 섹터
std::list<Character*> gSector[30][30];

void MakeCharacter()
{

}