#pragma once
#include "Character.h"
#include <map>
#include <list>

//-----------------------------------------------------------------
// 30�� �̻��� �ǵ��� �ƹ��� �޽��� ���ŵ� ���°�� ���� ����.
//-----------------------------------------------------------------
#define dfNETWORK_PACKET_RECV_TIMEOUT	30000

// ����
#define dfPACKET_DIR_L							0
#define dfPACKET_DIR_R							1

//-----------------------------------------------------------------
// ȭ�� �̵� ����.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400

//---------------------------------------------------------------
// ���ݹ���.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20


//---------------------------------------------------------------
// ���� ������.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3


//-----------------------------------------------------------------
// ĳ���� �̵� �ӵ�   // 25fps ���� �̵��ӵ�
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X	6	// 3   50fps
#define dfSPEED_PLAYER_Y	4	// 2   50fps


//-----------------------------------------------------------------
// �̵� ����üũ ����
//-----------------------------------------------------------------
#define dfERROR_RANGE		50

extern std::map<unsigned int, Character*> gCharacterMap;

// ����� ĳ���� ����
extern std::list<Character*> gSector[30][30];