#include "Network.h"
#include "Console.h"
#include "Log.h"
#include "Game.h"
#include "GameInfo.h"
//#include "Profiler.h"

#pragma comment(lib, "winmm.lib")

int main()
{
	// 타이머 해상도 높이기
	timeBeginPeriod(1);
	LogFileInit();

	//LoadData();
	Network network;

	//ProfileInit();
	if (network.StartUp())
	{
		Log(const_cast<WCHAR*>(L"소켓 초기화 오류!\n"), LOG_LEVEL_DEBUG);
		return 0;
	}

	unsigned int curTime = timeGetTime();
	unsigned int ourTime = curTime;
	unsigned int frameTime = curTime;
	int Frame = 0;
	while (Shutdown == false)
	{
		//ProfileBegin(L"IOProcess");
		network.IOProcess();
		//ProfileEnd(L"IOProcess");

		// 업데이트는 게임의 로직
		// 로직 처리
		if (ourTime < curTime)
		{
			//ProfileBegin(L"Update");
			Update();
			//ProfileEnd(L"Update");
			Frame++;
			ourTime += WAIT;
		}
			
		// 키보드 입력을 통해서 서버를 제어할 경우 사용
		ServerControl();

		curTime = timeGetTime();
		if (curTime - frameTime >= 1000)
		{
			if(Frame != FPS)
				_LOG(LOG_LEVEL_DEBUG, L"Frame : %d", Frame);
			Frame = 0;
			frameTime = curTime;
		}
	}

	// 서버 종료 대기
	
	// 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤 쓴다.
	network.CleanUp();

	//ProfileDataOutText(L"WhatIsThis.txt");

	return 0;
}