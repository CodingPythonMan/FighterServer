#include "Network.h"
#include "Log.h"
#include "Protocol.h"
#include "Proxy.h"
#include "Stub.h"
#include "PacketControl.h"

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
		{
			FD_SET(iter->second->Socket, &wset);
		}
			
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

	// Select 한번 끝난 후 삭제.
	DeleteSessions();
}

void Network::CleanUp()
{
	closesocket(_listenSock);

	WSACleanup();
}

void Network::SelectSocket(SOCKET* socketSet, int sockCount, FD_SET* rsetPtr, FD_SET* wsetPtr)
{
	// 프레임 서버이므로 timeval 설정
	timeval time{ 0, 100 };
	int retval;

	// 실질적인 select 함수를 호출하여 Readset 과 Writeset 을 확인하는 함수
	retval = select(0, rsetPtr, wsetPtr, nullptr, &time);
	if (retval <= 0)
		return;

	// 리슨소켓 검사
	for (int i = 0; i < sockCount; i++)
	{
		bool disconnectFlag = false;
		if (FD_ISSET(socketSet[i], wsetPtr))
		{
			disconnectFlag = WriteProc(socketSet[i]);
		}
		
		if (disconnectFlag == false)
		{
			if (FD_ISSET(socketSet[i], rsetPtr))
			{
				if (socketSet[i] == _listenSock)
				{
					AcceptProc();
				}
				else
				{
					ReadProc(socketSet[i]);
				}	
			}
		}
	}
}

void Network::AcceptProc()
{
	while (1)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSock = accept(_listenSock, (SOCKADDR*)&clientAddr, &addrLen);

		if (clientSock == INVALID_SOCKET)
		{
			clientSock = GetLastError();

			if (clientSock == WSAEWOULDBLOCK)
			{
				break;
			}
			else if (clientSock == WSAECONNRESET)
			{
				continue;
			}
			else
			{
				_LOG(LOG_LEVEL_SYSTEM, L"[Accept Error] ErrorCode : %d", (int)clientSock);
			}
		}

		// 세션 생성
		Session* session = CreateSession(clientSock);

		WCHAR IP[16];
		InetNtop(AF_INET, &clientAddr.sin_addr, IP, 16);
		// Session 생성 알려주기
		_LOG(LOG_LEVEL_DEBUG, L"[Client Connect] ID : %d, IP : %s, Port : %d",
			_uniqueID, IP, ntohs(clientAddr.sin_port));

		// 캐릭터 생성
		Character* character = new Character(session, _uniqueID);

		// 1. 당사자에게 생성됐음을 알려주기
		Packet packet;
		mpCreateMyCharacter(&packet, _uniqueID, character->Direct,
			character->X, character->Y, character->HP);

		SendPacket_Unicast(session, &packet);

		// 2. 다른 사람에게 알려주기
		mpCreateOtherCharacter(&packet, _uniqueID, character->Direct,
			character->X, character->Y, character->HP);

		SendPacket_Around(session, &packet);

		// 3. 다른 사람 위치 받기
		SectorPos pos = character->Sector;

		std::list<Character*> characterList = gSector[pos.Y][pos.X];
		for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
		{
			character = *iter;
			if (character->SessionPtr == session)
				continue;

			mpCreateOtherCharacter(&packet, character->SessionID, character->Direct,
				character->X, character->Y, character->HP);
			SendPacket_Unicast(session, &packet);

			if (character->MoveDirect >= dfPACKET_MOVE_STOP)
				continue;

			mpMoveStart(&packet, character->SessionID, character->MoveDirect,
				character->X, character->Y);
			SendPacket_Unicast(session, &packet);
		}

		// 4. 접속 주위 8방향에 있는 다른 캐릭터 받기
		for (int i = 0; i < 8; i++)
		{
			int dX = pos.X + dx[i];
			int dY = pos.Y + dy[i];

			if (dX < 0 || dX >= dfSECTOR_MAX_X || dY < 0 || dY >= dfSECTOR_MAX_Y)
				continue;

			characterList = gSector[dY][dX];

			for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
			{
				character = *iter;
				mpCreateOtherCharacter(&packet, character->SessionID, character->Direct,
					character->X, character->Y, character->HP);
				SendPacket_Unicast(session, &packet);

				if (character->MoveDirect >= dfPACKET_MOVE_STOP)
					continue;

				mpMoveStart(&packet, character->SessionID, character->MoveDirect,
					character->X, character->Y);
				SendPacket_Unicast(session, &packet);
			}
		}

		_uniqueID++;
	}
}

void Network::ReadProc(SOCKET sock)
{
	Session* session = FindSession(sock);
	if (session == nullptr)
		return;

	int retval;
	int recvAvailableSize = session->RecvQ.DirectEnqueueSize();

	// Rear 가 끝에 도달
	if (recvAvailableSize == 0)
	{
		recvAvailableSize = session->RecvQ.GetFreeSize();
		if (recvAvailableSize == 0)
			return;
	}

	char* ptr = session->RecvQ.GetRearBufferPtr();
	retval = recv(session->Socket, ptr, recvAvailableSize, 0);
	
	if (retval == SOCKET_ERROR)
	{
		retval = GetLastError();

		if (retval == WSAECONNRESET || retval == WSAECONNABORTED)
		{
			DisconnectSession(session);
			return;
		}
		else
		{
			_LOG(LOG_LEVEL_SYSTEM, L"[Read Error] ErrorCode : %d", retval);
			return;
		}
	}
	else if (retval == 0)
	{
		// 리턴값이 0인 건 종료 표시
		// IO_PENDING 이 뜰거라면 SOCKET_ERROR -1 로 GetLastError 시 WOULDBLOCK
		// IO_PENDING 이 뜬다는 건, 수신 버퍼에서 읽어올 게 없다는 뜻.(수신버퍼 0)
		DisconnectSession(session);
	}

	session->RecvQ.MoveRear(retval);
	
	while (1)
	{
		// Header 만큼 모이지 않았다면, 다음에 진행해야 한다.
		if (session->RecvQ.GetUseSize() <= sizeof(st_PACKET_HEADER))
			break;

		st_PACKET_HEADER header;
		session->RecvQ.Peek((char*)&header, sizeof(st_PACKET_HEADER));
		if (session->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize)
			break;

		// 프로토콜 코드가 맞지 않다면 내보낸다.
		if (header.byCode != 0x89)
		{
			_LOG(LOG_LEVEL_ERROR, L"[Read Error] Header is not correct : %d", header.byCode);
			break;
		}

		// 마샬링
		Packet packet;
		session->RecvQ.MoveFront(sizeof(st_PACKET_HEADER));
		retval = session->RecvQ.Dequeue(packet.GetBufferPtr(), header.bySize);

		// 왔다면 시간 갱신
		session->LastRecvTime = timeGetTime();

		if (false == PacketProc(session, header.byType, &packet))
			break;
	}
}

bool Network::WriteProc(SOCKET sock)
{
	Session* session = FindSession(sock);

	if (session == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"%s", L"WriteProc => Cannot Find Session!");
		return true;
	}

	int retval;
	int sendAvailableSize = session->SendQ.DirectDequeueSize();
	
	// Front 가 끝에 도달
	if (sendAvailableSize == 0)
	{
		sendAvailableSize = session->SendQ.GetUseSize();
		if (sendAvailableSize == 0)
			return false;
	}

	char* ptr = session->SendQ.GetFrontBufferPtr();
	retval = send(session->Socket, ptr, sendAvailableSize, 0);

	if (retval == SOCKET_ERROR)
	{
		retval = GetLastError();

		// WSAECONNABORTED 클라쪽에서 끊은 연결
		// WSAECONNRESET 우리쪽에서 끊었을 수 있음
		if (retval == WSAECONNRESET || retval == WSAECONNABORTED)
		{
			DisconnectSession(session);
			return true;
		}
		else
		{
			DisconnectSession(session);
			_LOG(LOG_LEVEL_ERROR, L"[Write Error] ErrorCode : %d", retval);
			return true;
		}
	}

	// send 를 못할 상황일 때, MoveFront 를 하면 안된다.
	session->SendQ.MoveFront(retval);

	return false;
}

bool Network::PacketProc(Session* session, unsigned char packetType, Packet* packet)
{
	switch (packetType)
	{
	case dfPACKET_CS_MOVE_START:
		return Proc_MoveStart(session, packet);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return Proc_MoveStop(session, packet);
		break;
	case dfPACKET_CS_ATTACK1:
		return Proc_Attack001(session, packet);
		break;
	case dfPACKET_CS_ATTACK2:
		return Proc_Attack002(session, packet);
		break;
	case dfPACKET_CS_ATTACK3:
		return Proc_Attack003(session, packet);
		break;
	case dfPACKET_CS_ECHO:
		return Proc_Echo(session, packet);
		break;
	default:
		break;
	}

	return true;
}

Session* Network::FindSession(SOCKET socket)
{
	return _sessionMap[socket];
}

Session* Network::CreateSession(SOCKET socket)
{
	Session* session = new Session{ socket, _uniqueID };
	_sessionMap.insert({ socket, session });

	return session;
}

void Network::DeleteSessions()
{
	SOCKADDR_IN clientAddr;
	WCHAR IP[16];
	int addrLen = sizeof(clientAddr);
	for (auto iter = _deleteList.begin(); iter != _deleteList.end(); ++iter)
	{
		getpeername((*iter)->Socket, (SOCKADDR*)&clientAddr, &addrLen);
		InetNtop(AF_INET, &clientAddr.sin_addr, IP, 16);
		_LOG(LOG_LEVEL_DEBUG, L"[Client Disconnect] ID : %d, IP : %s, Port : %d",
			_uniqueID, IP, ntohs(clientAddr.sin_port));

		_sessionMap.erase((*iter)->Socket);
		delete FindCharacter((*iter)->SessionID);
		closesocket((*iter)->Socket);
		delete (*iter);
	}

	_deleteList.clear();
}