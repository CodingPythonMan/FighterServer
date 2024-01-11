#include "Stub.h"
#include "Log.h"
#include "Character.h"
#include <cmath>
#include "Send.h"
#include "Proxy.h"

bool Proc_MoveStart(Session* session, Packet* packet)
{
	unsigned char Direct;
	short X, Y;

	*packet >> Direct;
	*packet >> X;
	*packet >> Y;

	_LOG(LOG_LEVEL_DEBUG, L"MoveStart => SessionID: %d / Direction: %d / X: %d / Y: %d",
		session->SessionID, Direct, X, Y);

	// ID 로 그 캐릭터를 검색한다.
	Character* character = FindCharacter(session->SessionID);

	if (character == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"MoveStart => SessionID: %d Chracter Not Found!", session->SessionID);
		return false;
	}

	// 서버의 위치와 받은 패킷 위치 값이 너무 큰 차이가 난다면 싱크 패킷 보내어 좌표 보정
	// 지금 좌표는 간단한 구현을 목적으로 하고 있다.
	if (abs(character->X - X) > dfERROR_RANGE || abs(character->Y - Y) > dfERROR_RANGE)
	{
		mpSync(packet, character->SessionID, character->X, character->Y);
		SendPacket_Around(session, packet, true);

		X = character->X;
		Y = character->Y;
	}

	// 동작 변경
	character->Action = Direct;

	// 단순 방향 표시용
	character->MoveDirect = Direct;

	// 방향을 변경


	return true;
}