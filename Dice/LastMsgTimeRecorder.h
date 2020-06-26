#pragma once

#ifndef Last_Time_MsgReporter
#define Last_Time_MsgReporter

#include <string>
#include <iostream>   
#include <windows.h>
#include <map>
#include <set>

using namespace std;

extern bool LMTScanOn;

extern map<long long, long long> LMTList;
extern set<long long> LMTWhiteList;

struct LMTMsgType
{
	LMTMsgType(bool a, string b) :isEffective(a), LMTMsg(b)
	{
	}
	bool isEffective;
	string LMTMsg;
};

//void LTSLogRec(long long Target_ID, string Action);
void LastMsgTimeRecorder(long long target_id);
string WhiteListControl(long long LowerQQID, long long Target_ID, bool isWriteIn);
LMTMsgType RefrList(long long LowerQQID);
void LeaveGroupScan();
string LeaveGroupScanControl(bool SwitchMsg, long long LowerQQID);
string LMTRecReport(long long LowerQQID);
void ListPack(bool isPack, bool CleraMemory);

#endif // !Last_Time_MsgReporter
