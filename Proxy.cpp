#include "Proxy.h"
#include "Protocol.h"

void mpCreateMyCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 10;
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
	*packet << HP;
}

void mpCreateOtherCharacter(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y, char HP)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 10;
	header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
	*packet << HP;
}