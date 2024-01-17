#include "Network.h"
#include "Console.h"
#include "Log.h"
#include "Game.h"
#include "Profiler.h"

#pragma comment(lib, "winmm.lib")

#define WAIT 20

int main()
{
	// Ÿ�̸� �ػ� ���̱�
	timeBeginPeriod(1);
	LogFileInit();

	//LoadData();
	Network network;

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
		network.IOProcess();

		// ������Ʈ�� ������ ����
		// ���� ó��
		if (ourTime < curTime)
		{
			Update();
			Frame++;
			ourTime += WAIT;
		}
			
		// Ű���� �Է��� ���ؼ� ������ ������ ��� ���
		ServerControl();

		// ����͸� ������ ǥ��, ����, �����ϴ� ��� ���
		// Monitor();

		curTime = timeGetTime();
		if (curTime - frameTime >= 1000)
		{
			if(Frame != (1000 / WAIT))
				_LOG(LOG_LEVEL_DEBUG, L"Frame : %d", Frame);
			Frame = 0;
			frameTime = curTime;
		}
	}

	// ���� ���� ���
	
	// ������ �Ժη� �����ص� �ȵȴ�.
	// DB�� ������ �����ͳ� ��Ÿ ������ �� �ϵ��� ��� �������� Ȯ���� �� ����.
	network.CleanUp();

	ProfileDataOutText(L"WhatIsThis.txt");

	return 0;
}