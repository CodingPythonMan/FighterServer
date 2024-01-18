#include "Network.h"
#include "Log.h"
#include "Protocol.h"
#include "Proxy.h"
#include "Stub.h"
#include "PacketControl.h"
#include "Profiler.h"

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
			ProfileBegin(L"WriteProc");
			disconnectFlag = WriteProc(socketSet[i]);
			ProfileEnd(L"WriteProc");
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
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSock = accept(_listenSock, (SOCKADDR*)&clientAddr, &addrLen);
	if (clientSock == INVALID_SOCKET)
		return;

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
	Packet CreateMyChar;
	mpCreateMyCharacter(&CreateMyChar, _uniqueID, character->Direct,
		character->X, character->Y, character->HP);

	SendPacket_Unicast(session, &CreateMyChar);

	// 2. �ٸ� ������� �˷��ֱ�
	Packet CreateOtherChar;
	mpCreateOtherCharacter(&CreateOtherChar, _uniqueID, character->Direct,
		character->X, character->Y, character->HP);

	SendPacket_Around(session, &CreateOtherChar);

	// 3. �ٸ� ��� ��ġ �ޱ�
	SectorPos pos = FindSectorPos(session->SessionID);

	std::list<Character*> characterList = gSector[pos.Y][pos.X];
	for (auto iter = characterList.begin(); iter != characterList.end(); ++iter)
	{
		character = *iter;
		if (character->SessionPtr == session)
			continue;
		
		mpCreateOtherCharacter(&CreateOtherChar, character->SessionID, character->Direct,
			character->X, character->Y, character->HP);
		SendPacket_Unicast(session, &CreateOtherChar);
	}

	// 8���⿡ ���ؼ� Sector Send
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
			mpCreateOtherCharacter(&CreateOtherChar, character->SessionID, character->Direct,
				character->X, character->Y, character->HP);
			SendPacket_Unicast(session, &CreateOtherChar);
		}
	}

	_uniqueID++;
}

void Network::ReadProc(SOCKET sock)
{
	Session* session = FindSession(sock);
	if (session == nullptr)
		return;

	int retval;
	int recvAvailableSize = session->RecvQ.DirectEnqueueSize();
	char* ptr = session->RecvQ.GetRearBufferPtr();
	retval = recv(session->Socket, ptr, recvAvailableSize, 0);
	session->RecvQ.MoveRear(retval);

	if (retval == SOCKET_ERROR)
	{
		retval = GetLastError();

		if (retval == WSAEWOULDBLOCK)
			return;
		else if (retval == WSAECONNRESET)
		{
			DisconnectSession(session);
			return;
		}
	}
	else if (retval == 0)
	{
		DisconnectSession(session);
	}
	
	while (1)
	{
		// Peek ���� ���� ��������.r
		if (session->RecvQ.GetUseSize() <= sizeof(st_PACKET_HEADER))
			break;

		st_PACKET_HEADER header;
		session->RecvQ.Peek((char*)&header, sizeof(st_PACKET_HEADER));
		if (session->RecvQ.GetUseSize() < sizeof(st_PACKET_HEADER) + header.bySize)
			break;

		// �������� �ڵ尡 ���� �ʴٸ� ��������.
		if (header.byCode != 0x89)
			break;

		// ������
		Packet packet;
		retval = session->RecvQ.Dequeue(packet.GetBufferPtr(), sizeof(st_PACKET_HEADER) + header.bySize);
		packet.GetData((char*)&header, sizeof(st_PACKET_HEADER));

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
		
	while (1)
	{
		if (session->SendQ.GetUseSize() <= 0)
			break;

		int retval;
		int sendAvailableSize = session->SendQ.DirectDequeueSize();
		char* ptr = session->SendQ.GetFrontBufferPtr();
		retval = send(session->Socket, ptr, sendAvailableSize, 0);
		session->SendQ.MoveFront(retval);

		if (retval == SOCKET_ERROR)
		{
			retval = GetLastError();

			if (retval == WSAEWOULDBLOCK)
				break;
			else if (retval == WSAECONNRESET)
			{
				DisconnectSession(session);
				return true;
			}
		}
	}

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
	for (auto iter = _deleteList.begin(); iter != _deleteList.end(); ++iter)
	{
		_sessionMap.erase((*iter)->Socket);
		delete FindCharacter((*iter)->SessionID);
		closesocket((*iter)->Socket);
		delete (*iter);
	}

	_deleteList.clear();
}