#include "Network.h"
#include <conio.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

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

int main()
{
	// 타이머 해상도 높이기
	timeBeginPeriod(1);

	//LoadData();
	Network network;

	network.StartUp();
	while (Shutdown == false)
	{
		network.IOProcess();

		// 업데이트는 게임의 로직
		// 로직 처리
		//Update();

		// 키보드 입력을 통해서 서버를 제어할 경우 사용
		ServerControl();

		// 모니터링 정보를 표시, 저장, 전송하는 경우 사용
		// Monitor();
	}

	// 서버 종료 대기
	
	// 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤 쓴다.


	return 0;
}