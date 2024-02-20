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
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// ���Ͱ� �Ѿ���� �Ǵ��ϰ� �ٸ� ������ Character List �� ��� �� �� �Ǵ��ؾ��Ѵ�.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X >= X - dfATTACK1_RANGE_X && target->X <= attacker->X)
					&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK1_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X - dfATTACK1_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK1_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X - 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK1_RANGE_X && target->X <= attacker->X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK1_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK1_RANGE_X && target->X <= attacker->X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK1_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X > 0 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y-1][Sector.X-1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK1_RANGE_X && target->X <= attacker->X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
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
		}
		else
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X <= X + dfATTACK1_RANGE_X && target->X >= attacker->X)
					&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK1_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X + dfATTACK1_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK1_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X < dfSECTOR_MAX_X - 1)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK1_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK1_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK1_RANGE_X && target->X >= attacker->X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= attacker->Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK1_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X < dfSECTOR_MAX_X - 1 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK1_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK1_RANGE_Y && target->Y <= Y))
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
		}
	} while (false);
	
	mpAttack001(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_Attack002(Session* session, Packet* packet)
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
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// ���Ͱ� �Ѿ���� �Ǵ��ϰ� �ٸ� ������ Character List �� ��� �� �� �Ǵ��ؾ��Ѵ�.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X >= X - dfATTACK2_RANGE_X && target->X <= X)
					&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK2_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X - dfATTACK2_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK2_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X - 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK2_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK2_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X > 0 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X - 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK2_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						break;
					}
				}
			}
		}
		else
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X <= X + dfATTACK2_RANGE_X && target->X >= X)
					&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK2_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X + dfATTACK2_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK2_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X < dfSECTOR_MAX_X - 1)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK2_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK2_RANGE_X && target->X >= attacker->X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= attacker->Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X < dfSECTOR_MAX_X - 1 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK2_RANGE_X && target->X >= attacker->X)
						&& (target->Y >= Y - dfATTACK2_RANGE_Y && target->Y <= attacker->Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK2_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						break;
					}
				}
			}
		}
	} while (false);

	mpAttack002(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_Attack003(Session* session, Packet* packet)
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
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// ���Ͱ� �Ѿ���� �Ǵ��ϰ� �ٸ� ������ Character List �� ��� �� �� �Ǵ��ؾ��Ѵ�.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X >= X - dfATTACK3_RANGE_X && target->X <= X)
					&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK3_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X - dfATTACK3_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK3_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X - 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK3_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK3_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X > 0 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X - 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X >= X - dfATTACK3_RANGE_X && target->X <= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						break;
					}
				}
			}
		}
		else
		{
			// �켱 ���� ���Ϳ��� üũ
			std::list<Character*>& characterList = gSector[Sector.Y][Sector.X];
			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				Character* target = *iter;
				if ((target->X <= X + dfATTACK3_RANGE_X && target->X >= X)
					&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
				{
					if (target == attacker)
						continue;

					target->OnDamage(dfATTACK3_DAMAGE);
					mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
					SendPacket_Around(target->SessionPtr, packet, true);

					if (target->IsDead())
						DisconnectSession(target->SessionPtr);

					attackSuccess = true;
					break;
				}
			}

			if (attackSuccess == true)
				break;

			exceedX = ((X + dfATTACK3_RANGE_X) / dfSECTOR_MAX_X != attacker->Sector.X);
			exceedY = ((Y - dfATTACK3_RANGE_Y) / dfSECTOR_MAX_Y != attacker->Sector.Y);

			if (exceedX == true && Sector.X < dfSECTOR_MAX_X - 1)
			{
				std::list<Character*>& characterList = gSector[Sector.Y][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK3_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedY == true && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK3_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						attackSuccess = true;
						break;
					}
				}
			}

			if (attackSuccess == true)
				break;

			if (exceedX == true && exceedY == true && Sector.X < dfSECTOR_MAX_X - 1 && Sector.Y > 0)
			{
				std::list<Character*>& characterList = gSector[Sector.Y - 1][Sector.X + 1];
				for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
				{
					Character* target = *iter;
					if ((target->X <= X + dfATTACK3_RANGE_X && target->X >= X)
						&& (target->Y >= Y - dfATTACK3_RANGE_Y && target->Y <= Y))
					{
						if (target == attacker)
							continue;

						target->OnDamage(dfATTACK3_DAMAGE);
						mpDamage(packet, attacker->SessionID, target->SessionID, target->HP);
						SendPacket_Around(target->SessionPtr, packet, true);

						if (target->IsDead())
							DisconnectSession(target->SessionPtr);

						break;
					}
				}
			}
		}
	} while (false);

	mpAttack003(packet, session->SessionID, Direct, X, Y);
	SendPacket_Around(session, packet);

	return true;
}

bool Proc_Echo(Session* session, Packet* packet)
{
	unsigned int time;
	*packet >> time;
	mpEcho(packet, time);
	SendPacket_Unicast(session, packet);

	return true;
}