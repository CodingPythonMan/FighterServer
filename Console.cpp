#include "Console.h"

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
			wprintf(L"Control Mode : Press 1 - Log Level Debug \n");
			wprintf(L"Control Mode : Press 2 - Log Level Error \n");
			wprintf(L"Control Mode : Press 3 - Log Level System \n");
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
		// �α� ���� ����
		if (L'1' == ControlKey && ControlMode)
		{
			wprintf(L"Control : Log Level - Debug\n");
			gLogLevel = LOG_LEVEL_DEBUG;
		}

		if (L'2' == ControlKey && ControlMode)
		{
			wprintf(L"Control : Log Level - Error\n");
			gLogLevel = LOG_LEVEL_ERROR;
		}

		if (L'3' == ControlKey && ControlMode)
		{
			wprintf(L"Control : Log Level - System\n");
			gLogLevel = LOG_LEVEL_SYSTEM;
		}
	}
}