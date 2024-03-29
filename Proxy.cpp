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

void mpDeleteCharacter(Packet* packet, unsigned int SessionID)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 4;
	header.byType = dfPACKET_SC_DELETE_CHARACTER;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
}

void mpMoveStart(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_MOVE_START;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
}

void mpMoveStop(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_MOVE_STOP;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
}

void mpAttack001(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK1;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
}

void mpAttack002(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK2;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
}

void mpAttack003(Packet* packet, unsigned int SessionID, unsigned char Direct, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_ATTACK3;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << Direct;
	*packet << X;
	*packet << Y;
}

void mpDamage(Packet* packet, unsigned int AttackID, unsigned int DamageID, char Damage)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 9;
	header.byType = dfPACKET_SC_DAMAGE;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << AttackID;
	*packet << DamageID;
	*packet << Damage;
}

void mpSync(Packet* packet, unsigned int SessionID, short X, short Y)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 8;
	header.byType = dfPACKET_SC_SYNC;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << SessionID;
	*packet << X;
	*packet << Y;
}

void mpEcho(Packet* packet, unsigned int Time)
{
	st_PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = 4;
	header.byType = dfPACKET_SC_ECHO;
	packet->Clear();
	packet->PutData((char*)&header, sizeof(st_PACKET_HEADER));
	*packet << Time;
}