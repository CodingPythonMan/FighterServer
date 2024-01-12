#include "Send.h"
#include "Character.h"

int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

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

void SendPacket_Broadcast(Session* session, Packet* packet)
{
	// 에코 메세지 쏠 떄, 필요할 수 있음.
}