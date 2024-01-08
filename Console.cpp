#include "Game.h"
#include <conio.h>
#include <stdio.h>
#include <Windows.h>

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
	}
}