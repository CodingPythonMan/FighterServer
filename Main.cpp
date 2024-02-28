#include "Network.h"
#include "Console.h"
#include "Log.h"
#include "Game.h"
#include "GameInfo.h"
//#include "Profiler.h"

#pragma comment(lib, "winmm.lib")

int main()
{
	// Ÿ�̸� �ػ� ���̱�
	timeBeginPeriod(1);
	LogFileInit();

	//LoadData();
	Network network;

	//ProfileInit();
	if (network.StartUp())
	{
		Log(const_cast<WCHAR*>(L"���� �ʱ�ȭ ����!\n"), LOG_LEVEL_DEBUG);
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

		// ������Ʈ�� ������ ����
		// ���� ó��
		if (ourTime < curTime)
		{
			//ProfileBegin(L"Update");
			Update();
			//ProfileEnd(L"Update");
			Frame++;
			ourTime += WAIT;
		}
			
		// Ű���� �Է��� ���ؼ� ������ ������ ��� ���
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

	// ���� ���� ���
	
	// ������ �Ժη� �����ص� �ȵȴ�.
	// DB�� ������ �����ͳ� ��Ÿ ������ �� �ϵ��� ��� �������� Ȯ���� �� ����.
	network.CleanUp();

	//ProfileDataOutText(L"WhatIsThis.txt");

	return 0;
}