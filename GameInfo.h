#pragma once

#define WAIT 20
#define FPS (1000 / WAIT)

//-----------------------------------------------------------------
// 30초 이상이 되도록 아무런 메시지 수신도 없는경우 접속 끊음.
//-----------------------------------------------------------------
#define dfNETWORK_PACKET_RECV_TIMEOUT	30000

#define MONITOR_TIME 60