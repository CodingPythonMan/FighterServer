#include "Network.h"
#include "Console.h"
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

	if (network.StartUp())
	{
		Log(const_cast<WCHAR*>(L"���� �ʱ�ȭ ����!\n"), LOG_LEVEL_DEBUG);
		return 0;
	}

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

		Frame++;

		curTime = timeGetTime();
		if (curTime - frameTime >= 1000)
		{
			if(Frame > (1000 / WAIT) && Frame > (1000 / WAIT + 1))
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