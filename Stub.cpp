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

	// ID �� �� ĳ���͸� �˻��Ѵ�.
	Character* character = FindCharacter(session->SessionID);

	if (character == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"MoveStart => SessionID: %d Chracter Not Found!", session->SessionID);
		return false;
	}

	// ������ ��ġ�� ���� ��Ŷ ��ġ ���� �ʹ� ū ���̰� ���ٸ� ��ũ ��Ŷ ������ ��ǥ ����
	// ���� ��ǥ�� ������ ������ �������� �ϰ� �ִ�.
	if (abs(character->X - X) > dfERROR_RANGE || abs(character->Y - Y) > dfERROR_RANGE)
	{
		mpSync(packet, character->SessionID, character->X, character->Y);
		SendPacket_Around(session, packet, true);

		X = character->X;
		Y = character->Y;
	}

	// ���� ����
	character->Action = Direct;

	// �ܼ� ���� ǥ�ÿ�
	character->MoveDirect = Direct;

	// ������ ����


	return true;
}