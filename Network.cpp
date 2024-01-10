#include "Network.h"
#include "Log.h"
#include "Protocol.h"
#include "Proxy.h"
#include "Stub.h"
#include "Character.h"

Network::Network()
{
	_listenSock = NULL;
	_uniqueID = 0;
}

Network::~Network()
{
}

bool Network::StartUp()
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return true;

	// 리슨 소켓 셋팅
	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
		return true;

	// bind
	SOCKADDR_IN listenAddr;
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(SERVER_PORT);
	retval = bind(_listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr));
	if (retval == SOCKET_ERROR)
		return true;

	// linger 옵션 설정
	linger Linger;
	Linger.l_onoff = 1;
	Linger.l_linger = 0;
	setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(Linger));

	// 논블라킹 소켓으로 전환
	unsigned long on = 1;
	retval = ioctlsocket(_listenSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
		return true;

	// listen
	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return true;

	// 나중 파일로까지 남기고 싶다면 로그 레벨을 승격
	wprintf(L"[Listen Status] : OK\n");

	return false;
}

void Network::IOProcess()
{
	// FD_SET 구조체 셋팅
	FD_SET rset, wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(_listenSock, &rset);
	
	// Select 에 필요한 변수들 설정
	int Count = 1;
	SOCKET SockSet[FD_SETSIZE];
	SockSet[0] = _listenSock;
	
	for (auto iter = _sessionMap.begin(); iter != _sessionMap.end(); ++iter)
	{
		SockSet[Count] = iter->second->Socket;
		FD_SET(iter->second->Socket, &rset);
		Count++;
		
		if (iter->second->SendQ.GetUseSize() > 0)
			FD_SET(iter->second->Socket, &wset);

		// Select 최대치 도달. 만들어진 테이블 정보로 select 호출 후 정리
		if (Count >= FD_SETSIZE)
		{
			SelectSocket(SockSet, Count, &rset, &wset);
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			Count = 0;
		}
	}

	SelectSocket(SockSet, Count, &rset, &wset);
}

void Network::SelectSocket(SOCKET* socketSet, int sockCount, FD_SET* rsetPtr, FD_SET* wsetPtr)
{
	// 프레임 서버이므로 timeval 설정
	timeval time{ 0, 0 };
	int retval;

	// 실질적인 select 함수를 호출하여 Readset 과 Writeset 을 확인하는 함수
	retval = select(0, rsetPtr, wsetPtr, nullptr, &time);
	if (retval <= 0)
		return;

	// 리슨소켓 검사
	for (int i = 0; i < sockCount; i++)
	{
		if (FD_ISSET(socketSet[i], rsetPtr))
		{
			if (socketSet[i] == _listenSock)
				AcceptProc();
			else
				ReadProc(socketSet[i]);
		}

		if (FD_ISSET(socketSet[i], wsetPtr))
			WriteProc(socketSet[i]);
	}
}

void Network::AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSock = accept(_listenSock, (SOCKADDR*)&clientAddr, &addrLen);
	if (clientSock == INVALID_SOCKET)
		return;

	Session* session = new Session{ clientSock, _uniqueID };
	_sessionMap.insert({clientSock, session});

	WCHAR IP[16];
	InetNtop(AF_INET, &clientAddr.sin_addr, IP, 16);
	// Session 생성 알려주기
	wprintf(L"[Client Connect] ID : %d, IP : %s, Port : %d\n", _uniqueID, IP, ntohs(clientAddr.sin_port));

	// 캐릭터 생성
	Character* character = new Character;
	gCharacterMap.insert({ _uniqueID, character });

	// 1. 당사자에게 생성됐음을 알려주기
	Packet CreateMyChar;
	mpCreateMyCharacter(&CreateMyChar, _uniqueID, character->GetDirect(),
		character->GetX(), character->GetY(), character->GetHP());

	SendPacket_Unicast(session, &CreateMyChar);

	// 2. 다른 사람에게 알려주기
	Packet CreateOtherChar;
	mpCreateOtherCharacter(&CreateOtherChar, _uniqueID, character->GetDirect(),
		character->GetX(), character->GetY(), character->GetHP());

	SendPacket_Around(session, &CreateOtherChar);
}

void Network::ReadProc(SOCKET sock)
{

}

void Network::WriteProc(SOCKET sock)
{
	Session* session = _sessionMap[sock];
	// 해당 에러 소스에 추가 필요.
	if (session == nullptr)
		return;

	while (1)
	{
		if (session->SendQ.GetUseSize() <= 0)
			return;

		int retval;
		int sendAvailableSize = session->SendQ.DirectDequeueSize();
		char* ptr = session->SendQ.GetFrontBufferPtr();
		retval = send(session->Socket, ptr, sendAvailableSize, 0);
		session->SendQ.MoveFront(retval);

		if (retval == SOCKET_ERROR)
		{
			retval = GetLastError();

			if (retval == WSAEWOULDBLOCK)
				return;
			else if (retval == WSAECONNRESET)
			{
				//Disconnect(session);
				return;
			}
		}
	}
}

bool Network::PacketProc(Session* session, unsigned char packetType, Packet* packet)
{
	switch (packetType)
	{
	case dfPACKET_CS_MOVE_START:
		return Proc_MoveStart(session, packet);
		break;
	}

	return true;
}

void Network::SendPacket_Unicast(Session* session, Packet* packet)
{
	if (session->SendQ.GetFreeSize() > packet->GetDataSize())
		session->SendQ.Enqueue((char*)packet->GetBufferPtr(), packet->GetDataSize());
}

void Network::SendPacket_Around(Session* session, Packet* packet, bool me)
{
	Character* character = gCharacterMap[session->SessionID];
	SectorPos* pos = character->GetSectorPtr();
	//gSector[pos->Y][pos->X]
}