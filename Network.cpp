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
			SelectSocket(SockSet, &rset, &wset);
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			Count = 0;
		}
	}
}

void Network::SelectSocket(SOCKET* socketSet, FD_SET* rsetPtr, FD_SET* wsetPtr)
{
	// 프레임 서버이므로 timeval 설정
	timeval time{ 0, 0 };
	int retval;

	// 실질적인 select 함수를 호출하여 Readset 과 Writeset 을 확인하는 함수
	retval = select(0, rsetPtr, wsetPtr, nullptr, &time);
	if (retval <= 0)
		return;

	// 리슨소켓 검사
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
	// Session 생성 알려주기
	wprintf(L"[Client Connect] ID : %d, IP : %s, Port : %d\n", _uniqueID, IP, ntohs(clientAddr.sin_port));

	MakeCharacter();
}

void Network::ReadProc(SOCKET sock)
{

}

void Network::WriteProc(SOCKET sock)
{

}
