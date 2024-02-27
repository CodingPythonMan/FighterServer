#include "Console.h"

bool Shutdown = false;

void ServerControl()
{
	// 키보드 컨트롤 잠금, 풀림 변수
	static bool ControlMode = false;

	// L : 컨트롤 Lock / U : 컨트롤 Unlock / G : 서버 종료
	if (_kbhit())
	{
		WCHAR ControlKey = _getwch();

		// 키보드 제어 허용
		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			ControlMode = true;

			// 관련 키 도움말 출력
			wprintf(L"Control Mode : Press Q - Quit \n");
			wprintf(L"Control Mode : Press L - Key Lock \n");
			wprintf(L"Control Mode : Press 1 - Log Level Debug \n");
			wprintf(L"Control Mode : Press 2 - Log Level Error \n");
			wprintf(L"Control Mode : Press 3 - Log Level System \n");
		}

		// 키보드 제어 잠금
		if ((L'l' == ControlKey || L'L' == ControlKey) && ControlMode)
		{
			wprintf(L"Control Lock! Press U - Control Unlock\n");
			ControlMode = false;
		}

		// 키보드 제어 풀림 상태에서 특정 기능
		if ((L'q' == ControlKey || L'Q' == ControlKey) && ControlMode)
		{
			Shutdown = true;
		}

		// 나중에 추가할 것.
		// 로그 레벨 수정
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