#pragma once
#include "Session.h"
#include "Packet.h"
#include "Character.h"

bool Proc_MoveStart(Session* session, Packet* packet);
bool Proc_MoveStop(Session* session, Packet* packet);

bool Proc_Attack001(Session* session, Packet* packet);
bool Proc_Attack002(Session* session, Packet* packet);
bool Proc_Attack003(Session* session, Packet* packet);

bool Proc_Echo(Session* session, Packet* packet);