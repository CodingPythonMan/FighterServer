#include "Stub.h"
#include "Log.h"
#include "Character.h"
#include <cmath>
#include "PacketControl.h"
#include "Proxy.h"
#include "Sector.h"

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

		_LOG(LOG_LEVEL_ERROR, L"Start Sync => ID: %d, Before: (%d, %d), Client: (%d, %d), Direct: %d !", session->SessionID,
			character->X, character->Y, X, Y, character->MoveDirect);

		X = character->X;
		Y = character->Y;
	}

	// ���� ����
	character->Action = Direct;

	// �ܼ� ���� ǥ�ÿ�
	character->MoveDirect = Direct;

	// ������ ����
	switch (Direct)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		character->Direct = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		character->Direct = dfPACKET_MOVE_DIR_LL;
		break;
	}
	character->X = X;
	character->Y = Y;

	// ��ǥ�� �ణ ���� �����Ƿ� ���� ������Ʈ
	// ���Ͱ� ����� ��� Ŭ�� ���� ���
	if (SectorUpdateCharacter(character))
	{
		SectorUpdatePacket(character);
	}

	mpMoveStart(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_MoveStop(Session* session, Packet* packet)
{
	unsigned char Direct;
	short X, Y;

	*packet >> Direct;
	*packet >> X;
	*packet >> Y;

	_LOG(LOG_LEVEL_DEBUG, L"MoveStop => SessionID: %d / Direction: %d / X: %d / Y: %d",
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

		_LOG(LOG_LEVEL_ERROR, L"Stop Sync => ID: %d, Before: (%d, %d), Client: (%d, %d), Direct: %d !", session->SessionID,
			character->X, character->Y, X, Y, character->MoveDirect);

		X = character->X;
		Y = character->Y;
	}

	// ���� ����
	character->MoveDirect = dfPACKET_MOVE_STOP;

	// ������ ����
	character->Direct = Direct;
	character->X = X;
	character->Y = Y;

	// ��ǥ�� �ణ ���� �����Ƿ� ���� ������Ʈ
	// ���Ͱ� ����� ��� Ŭ�� ���� ���
	if (SectorUpdateCharacter(character))
	{
		SectorUpdatePacket(character);
	}

	mpMoveStop(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_Echo(Session* session, Packet* packet)
{
	session->LastRecvTime = timeGetTime();

	mpEcho(packet, session->LastRecvTime);
	SendPacket_Unicast(session, packet);

	return true;
}