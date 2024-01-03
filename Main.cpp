#include "Network.h"
#include <conio.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

bool Shutdown = false;

void ServerControl()
{
	// Ű���� ��Ʈ�� ���, Ǯ�� ����
	static bool ControlMode = false;

	// L : ��Ʈ�� Lock / U : ��Ʈ�� Unlock / G : ���� ����
	if (_kbhit())
	{
		WCHAR ControlKey = _getwch();

		// Ű���� ���� ���
		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			ControlMode = true;

			// ���� Ű ���� ���
			wprintf(L"Control Mode : Press Q - Quit \n");
			wprintf(L"Control Mode : Press L - Key Lock \n");
		}

		// Ű���� ���� ���
		if ((L'l' == ControlKey || L'L' == ControlKey) && ControlMode)
		{
			wprintf(L"Control Lock! Press U - Control Unlock\n");
			ControlMode = false;
		}

		// Ű���� ���� Ǯ�� ���¿��� Ư�� ���
		if ((L'q' == ControlKey || L'Q' == ControlKey) && ControlMode)
		{
			Shutdown = true;
		}

		// ���߿� �߰��� ��.
	}
}

int main()
{
	// Ÿ�̸� �ػ� ���̱�
	timeBeginPeriod(1);

	//LoadData();
	Network network;

	network.StartUp();
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
	}

	// ���� ���� ���
	
	// ������ �Ժη� �����ص� �ȵȴ�.
	// DB�� ������ �����ͳ� ��Ÿ ������ �� �ϵ��� ��� �������� Ȯ���� �� ����.


	return 0;
}