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

		_LOG(LOG_LEVEL_ERROR, L"Start Sync => ID: %d, Before: (%d, %d), Client: (%d, %d), Direct: %d !", session->SessionID,
			character->X, character->Y, X, Y, character->MoveDirect);

		X = character->X;
		Y = character->Y;
	}

	// 동작 변경
	character->Action = Direct;

	// 단순 방향 표시용
	character->MoveDirect = Direct;

	// 방향을 변경
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

	// 좌표가 약간 조절 됐으므로 섹터 업데이트
	// 섹터가 변경된 경우 클라 정보 쏘기
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

		_LOG(LOG_LEVEL_ERROR, L"Stop Sync => ID: %d, Before: (%d, %d), Client: (%d, %d), Direct: %d !", session->SessionID,
			character->X, character->Y, X, Y, character->MoveDirect);

		X = character->X;
		Y = character->Y;
	}

	// 동작 변경
	character->MoveDirect = dfPACKET_MOVE_STOP;

	// 방향을 변경
	character->Direct = Direct;
	character->X = X;
	character->Y = Y;

	// 좌표가 약간 조절 됐으므로 섹터 업데이트
	// 섹터가 변경된 경우 클라 정보 쏘기
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

	// 같은 섹터 안에서만 판단하면 안된다.
	// 공격을 꼭지점화 해서 우리 섹터를 벗어나는지 판단 후
	// 넘어가는 공격이 있으면 해당 섹터 리스트도 공격 범위를 체크 한다.

	// 우선 같은 섹터 체크
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// 섹터가 넘어가는지 판단하고 다른 섹터의 Character List 를 들고 온 후 판단해야한다.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// 우선 같은 섹터에서 체크
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
			// 우선 같은 섹터에서 체크
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

	// 같은 섹터 안에서만 판단하면 안된다.
	// 공격을 꼭지점화 해서 우리 섹터를 벗어나는지 판단 후
	// 넘어가는 공격이 있으면 해당 섹터 리스트도 공격 범위를 체크 한다.

	// 우선 같은 섹터 체크
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// 섹터가 넘어가는지 판단하고 다른 섹터의 Character List 를 들고 온 후 판단해야한다.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// 우선 같은 섹터에서 체크
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
			// 우선 같은 섹터에서 체크
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

	// 같은 섹터 안에서만 판단하면 안된다.
	// 공격을 꼭지점화 해서 우리 섹터를 벗어나는지 판단 후
	// 넘어가는 공격이 있으면 해당 섹터 리스트도 공격 범위를 체크 한다.

	// 우선 같은 섹터 체크
	bool exceedX;
	bool exceedY;

	bool attackSuccess = false;

	do {
		// 섹터가 넘어가는지 판단하고 다른 섹터의 Character List 를 들고 온 후 판단해야한다.
		if (Direct == dfPACKET_MOVE_DIR_LL)
		{
			// 우선 같은 섹터에서 체크
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
			// 우선 같은 섹터에서 체크
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