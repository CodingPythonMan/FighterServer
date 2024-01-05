#include "Network.h"
#include "Game.h"
#include "Log.h"

#pragma comment(lib, "winmm.lib")

#define WAIT 20

int main()
{
	// Ÿ�̸� �ػ� ���̱�
	timeBeginPeriod(1);
	LogFileInit();

	//LoadData();
	Network network;

	network.StartUp();

	unsigned int tick = 0;
	unsigned int curTime = timeGetTime();
	unsigned int ourTime = curTime;
	unsigned int frameTime = curTime;
	int Frame = 0;
	while (Shutdown == false)
	{
		network.IOProcess();

		// ������Ʈ�� ������ ����
		// ���� ó��
		//Update();

		// Ű���� �Է��� ���ؼ� ������ ������ ��� ���
		ServerControl();

		// ����͸� ������ ǥ��, ����, �����ϴ� ��� ���
		// Monitor();

		// ������ ��� �ʿ�
		Frame++;

		curTime = timeGetTime();
		if (curTime - frameTime >= 1000)
		{
			_LOG(LOG_LEVEL_DEBUG, L"Frame : %d", Frame);
			Frame = 0;
			frameTime = curTime;
		}
		tick = curTime - ourTime;
		ourTime += WAIT;

		if (tick <= WAIT)
		{
			Sleep(WAIT - tick);
		}
	}

	// ���� ���� ���
	
	// ������ �Ժη� �����ص� �ȵȴ�.
	// DB�� ������ �����ͳ� ��Ÿ ������ �� �ϵ��� ��� �������� Ȯ���� �� ����.


	return 0;
}