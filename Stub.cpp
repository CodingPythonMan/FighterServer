#include "Stub.h"
#include "Log.h"
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

bool Proc_Attack001(Session* session, Packet* packet)
{
	unsigned char Direct;
	short X, Y;

	*packet >> Direct;
	*packet >> X;
	*packet >> Y;

	Character* attacker = FindCharacter(session->SessionID);
	attacker->X = X;
	attacker->Y = Y;
	attacker->Direct = Direct;

	SectorPos Sector = attacker->Sector;

	// ���� ���� �ȿ����� �Ǵ��ϸ� �ȵȴ�.
	// ������ ������ȭ �ؼ� �츮 ���͸� ������� �Ǵ� ��
	// �Ѿ�� ������ ������ �ش� ���� ����Ʈ�� ���� ������ üũ �Ѵ�.

	// �켱 ���� ���� üũ
	bool exceedX = ((X - dfATTACK1_RANGE_X) / dfSECTOR_MAX_X == attacker->Sector.X);
	bool exceedY = ((Y + dfATTACK1_RANGE_Y) / dfSECTOR_MAX_Y == attacker->Sector.Y);
	
	bool AttackSucceed;

	if (Direct == dfPACKET_MOVE_DIR_LL)
	{
		if (exceedX == true && exceedY == true)
		{
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X >= X - dfATTACK1_RANGE_X && attacker->X >= target->X)
					&& (target->Y <= Y + dfATTACK1_RANGE_Y && target->Y >= attacker->Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK1_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					break;
				}
			}
		}
		else if (exceedX == true && exceedY == false)
		{
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X >= X - dfATTACK1_RANGE_X && attacker->X >= target->X)
					&& (target->Y <= Y + dfATTACK1_RANGE_Y && target->Y >= attacker->Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK1_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					break;
				}
			}
		}
		else if (exceedX == false && exceedY == true)
		{

		}
		else
		{

		}

		
	}
	else
	{
		std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
		for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
		{
			Character* target = *iter;
			if ((target->X >= X - dfATTACK1_RANGE_X && attacker->X >= target->X)
				&& (target->Y <= Y + dfATTACK1_RANGE_Y && target->Y >= attacker->Y))
			{
				if (target == attacker)
					continue;

				target->OnDamage(dfATTACK1_DAMAGE);
				mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
				SendPacket_Around(target->SessionPtr, packet, true);

				if (target->IsDead())
					DisconnectSession(target->SessionPtr);
			}
		}
	}

	mpAttack001(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_Attack002(Session* session, Packet* packet)
{
	return false;
}

bool Proc_Attack003(Session* session, Packet* packet)
{
	return false;
}

bool Proc_Echo(Session* session, Packet* packet)
{
	unsigned int time;
	*packet >> time;
	mpEcho(packet, time);
	SendPacket_Unicast(session, packet);

	return true;
}