#include "PacketControl.h"
#include "Character.h"
#include "Log.h"
#include "Proxy.h"

int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

std::list<Session*> _deleteList;

void SendPacket_SectorOne(int sectorX, int sectorY, Packet* packet, Session* exceptSession)
{
	if (sectorX < 0 || sectorX >= dfSECTOR_MAX_X || sectorY < 0 || sectorY >= dfSECTOR_MAX_Y)
		return;

	std::list<Character*> characterList = gSector[sectorY][sectorX];
	for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
	{
		if ((*iter)->SessionPtr == exceptSession)
			continue;

		SendPacket_Unicast((*iter)->SessionPtr, packet);
	}
}

void SendPacket_Unicast(Session* session, Packet* packet)
{
	if (session->SendQ.GetFreeSize() > packet->GetDataSize())
		session->SendQ.Enqueue((char*)packet->GetBufferPtr(), packet->GetDataSize());
	else
	{
		_LOG(LOG_LEVEL_ERROR, L"%s", L"SendPacket => RingBuffer Full Error!");
		// SendQ 를 초과하면 연결을 바로 끊는다.
		DisconnectSession(session);
	}
}

void SendPacket_Around(Session* session, Packet* packet, bool me)
{
	SectorPos pos = FindSectorPos(session->SessionID);

	std::list<Character*> characterList = gSector[pos.Y][pos.X];
	for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
	{
		if (me == false && (*iter)->SessionPtr == session)
			continue;

		SendPacket_Unicast((*iter)->SessionPtr, packet);
	}

	// 8방향에 대해서 Sector Send
	for (int i = 0; i < 8; i++)
	{
		int dX = pos.X + dx[i];
		int dY = pos.Y + dy[i];

		if (dX < 0 || dX >= dfSECTOR_MAX_X || dY < 0 || dY >= dfSECTOR_MAX_Y)
			continue;

		characterList = gSector[dY][dX];

		for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
		{
			SendPacket_Unicast((*iter)->SessionPtr, packet);
		}
	}
}

void DisconnectSession(Session* session)
{
	Packet Delete;
	mpDeleteCharacter(&Delete, session->SessionID);
	SendPacket_Around(session, &Delete);

	_deleteList.push_back(session);
}