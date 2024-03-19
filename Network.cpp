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

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return true;

	// ���� ���� ����
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

	// linger �ɼ� ����
	linger Linger;
	Linger.l_onoff = 1;
	Linger.l_linger = 0;
	setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(Linger));

	// ����ŷ �������� ��ȯ
	unsigned long on = 1;
	retval = ioctlsocket(_listenSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
		return true;

	// listen
	retval = listen(_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		return true;

	// ���� ���Ϸα��� ����� �ʹٸ� �α� ������ �°�
	wprintf(L"[Listen Status] : OK\n");

	return false;
}

void Network::IOProcess()
{
	// FD_SET ����ü ����
	FD_SET rset, wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(_listenSock, &rset);
	
	// Select �� �ʿ��� ������ ����
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
			
		// Select �ִ�ġ ����. ������� ���̺� ������ select ȣ�� �� ����
		if (Count >= FD_SETSIZE)
		{
			SelectSocket(SockSet, Count, &rset, &wset);
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			Count = 0;
		}
	}

	SelectSocket(SockSet, Count, &rset, &wset);

	// Select �ѹ� ���� �� ����.
	DeleteSessions();
}

void Network::CleanUp()
{
	closesocket(_listenSock);

	WSACleanup();
}

void Network::SelectSocket(SOCKET* socketSet, int sockCount, FD_SET* rsetPtr, FD_SET* wsetPtr)
{
	// ������ �����̹Ƿ� timeval ����
	timeval time{ 0, 100 };
	int retval;

	// �������� select �Լ��� ȣ���Ͽ� Readset �� Writeset �� Ȯ���ϴ� �Լ�
	retval = select(0, rsetPtr, wsetPtr, nullptr, &time);
	if (retval <= 0)
		return;

	// �������� �˻�
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

		// ���� ����
		Session* session = CreateSession(clientSock);

		WCHAR IP[16];
		InetNtop(AF_INET, &clientAddr.sin_addr, IP, 16);
		// Session ���� �˷��ֱ�
		_LOG(LOG_LEVEL_DEBUG, L"[Client Connect] ID : %d, IP : %s, Port : %d",
			_uniqueID, IP, ntohs(clientAddr.sin_port));

		// ĳ���� ����
		Character* character = new Character(session, _uniqueID);

		// 1. ����ڿ��� ���������� �˷��ֱ�
		Packet packet;
		mpCreateMyCharacter(&packet, _uniqueID, character->Direct,
			character->X, character->Y, character->HP);

		SendPacket_Unicast(session, &packet);

		// 2. �ٸ� ������� �˷��ֱ�
		mpCreateOtherCharacter(&packet, _uniqueID, character->Direct,
			character->X, character->Y, character->HP);

		SendPacket_Around(session, &packet);

		// 3. �ٸ� ��� ��ġ �ޱ�
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

		// 4. ���� ���� 8���⿡ �ִ� �ٸ� ĳ���� �ޱ�
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

	// Rear �� ���� ����
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
		// ���ϰ��� 0�� �� ���� ǥ��
		// IO_PENDING �� ��Ŷ�� SOCKET_ERROR -1 �� GetLastError �� WOULDBLOCK
		// IO_PENDING �� ��ٴ� ��, ���� ���ۿ��� �о�� �� ���ٴ� ��.(���Ź��� 0)
		DisconnectSession(session);
	}

	session->RecvQ.MoveRear(retval);
	
	while (1)
	{
		// Header ��ŭ ������ �ʾҴٸ�, ������ �����ؾ� �Ѵ�.
		if (session->RecvQ.GetUseSize() <= sizeof(st_PACKET_HEADER))
			break;

		st_PACKET_HEADER header;
		session->RecvQ.Peek((char*)&header, sizeof(st_PACKET_HEADER));
		if (session->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize)
			break;

		// �������� �ڵ尡 ���� �ʴٸ� ��������.
		if (header.byCode != 0x89)
		{
			_LOG(LOG_LEVEL_ERROR, L"[Read Error] Header is not correct : %d", header.byCode);
			break;
		}

		// ������
		Packet packet;
		session->RecvQ.MoveFront(sizeof(st_PACKET_HEADER));
		retval = session->RecvQ.Dequeue(packet.GetBufferPtr(), header.bySize);

		// �Դٸ� �ð� ����
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
	
	// Front �� ���� ����
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

		// WSAECONNABORTED Ŭ���ʿ��� ���� ����
		// WSAECONNRESET �츮�ʿ��� ������ �� ����
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

	// send �� ���� ��Ȳ�� ��, MoveFront �� �ϸ� �ȵȴ�.
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