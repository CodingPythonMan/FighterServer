#pragma once
#include "Session.h"
#include "Packet.h"

bool Proc_MoveStart(Session* session, Packet* packet);
bool Proc_MoveStop(Session* session, Packet* packet);