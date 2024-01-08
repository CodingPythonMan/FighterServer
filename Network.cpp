#include "Network.h"
#include "Log.h"
#include "Game.h"

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
			FD_SET(iter->second->Socket, &wset);

		// Select �ִ�ġ ����. ������� ���̺� ������ select ȣ�� �� ����
		if (Count >= FD_SETSIZE)
		{
			SelectSocket(SockSet, &rset, &wset);
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			Count = 0;
		}
	}
}

void Network::SelectSocket(SOCKET* socketSet, FD_SET* rsetPtr, FD_SET* wsetPtr)
{
	// ������ �����̹Ƿ� timeval ����
	timeval time{ 0, 0 };
	int retval;

	// �������� select �Լ��� ȣ���Ͽ� Readset �� Writeset �� Ȯ���ϴ� �Լ�
	retval = select(0, rsetPtr, wsetPtr, nullptr, &time);
	if (retval <= 0)
		return;

	// �������� �˻�
	for (int i = 0; i < retval; i++)
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

	Session* session = new Session;
	session->Socket = clientSock;
	session->SessionID = _uniqueID;

	WCHAR IP[16];
	InetNtop(AF_INET, &clientAddr.sin_addr, IP, 16);
	// Session ���� �˷��ֱ�
	wprintf(L"[Client Connect] ID : %d, IP : %s, Port : %d\n", _uniqueID, IP, ntohs(clientAddr.sin_port));

	MakeCharacter();
}

void Network::ReadProc(SOCKET sock)
{

}

void Network::WriteProc(SOCKET sock)
{

}
