#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <string>
#include <list>

#include <time.h>/*以下为计时模块*/
#include <Windows.h>

#include "cqdefine.h"
#include "CQAPI_EX.h"
#include "CDMainCirculate.h"
#include "MasterData.h"
#include "MemoRecorder.h"

#pragma comment( lib,"winmm.lib" )

using namespace std;
using namespace CQ;

/*备忘录开关*/
bool MemoScanOn = 1;

map<long long, MemoType> MemoMap;

string MemoRecorder(long long LowerQQID, string MemoIn, MemoRecoEnum EnumSwitch, unsigned int ClearWhere)
{
	switch (EnumSwitch)
	{
	case MemoRecoEnum::New:
	{
		if (MemoMap.count(LowerQQID))
		{
			MemoType TemMemo(MemoMap[LowerQQID]);
			TemMemo.Memo.clear();
			TemMemo += MemoIn;
			MemoMap[LowerQQID] = TemMemo;
		}
		else
			MemoMap[LowerQQID] = MemoType(MemoIn);
		return "已将该条目记录进新的备忘录";
	}
	case MemoRecoEnum::Add:
	{
		if (MemoMap.count(LowerQQID))
		{
			MemoType TemMemoType(MemoMap[LowerQQID]);
			TemMemoType += MemoIn;
			MemoMap[LowerQQID] = TemMemoType;
		}
		else
			MemoMap[LowerQQID] = MemoType(MemoIn);
		return "已将该条目记录进备忘录";
	}
	case MemoRecoEnum::Clear:
	{
		if (!MemoMap.count(LowerQQID))
			return "您还未在这里设置过备忘录";
		if (ClearWhere)
		{
			MemoType TemMemo(MemoMap[LowerQQID]);
			if (TemMemo.Memo.size() < ClearWhere)
				return "无法删除该条目！您只记录了" + to_string(TemMemo.Memo.size()) + "条备忘";
			TemMemo -= ClearWhere;
			if (TemMemo.Memo.empty())
				MemoMap.erase(LowerQQID);
			else
				MemoMap[LowerQQID] = TemMemo;
			return "已将该条目删除";
		}
		else
		{
			MemoMap.erase(LowerQQID);
			return "已将所有备忘删除";
		}
	}
	default:
		return "未知错误";
	}
}

string MemoRecorderGet(long long LowerQQID)
{
	if (MemoMap.count(LowerQQID))
		return "记录的备忘如下" + MemoMap[LowerQQID].Display();
	else
		return "还未在这里记录过备忘";
}

void MemoAlarmScan(SYSTEMTIME NowTime)
{
	map<long long, MemoType>::iterator MemoScan = MemoMap.begin();
	while (MemoScan != MemoMap.end())
	{
		string TimeRepo = to_string(NowTime.wMonth) + "月" + to_string(NowTime.wDay) + "日周" + to_string(NowTime.wDayOfWeek == 0 ? 7 : NowTime.wDayOfWeek) + "," + to_string(NowTime.wHour) + "点";
		if (MemoScan->second.AlarmOn && MemoScan->second.AlarmTime == NowTime.wHour)
		{
			if ((NowTime.wHour >= 0) && (NowTime.wHour < 5))
			{
				string MemoAlarmReply = "现在时间是...呼(哈欠）...\n" + TimeRepo + "\n这么晚还没睡么？你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 5) && (NowTime.wHour < 8))
			{
				string MemoAlarmReply = "早，现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 8) && (NowTime.wHour < 11))
			{
				string MemoAlarmReply = "上午好，现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 11) && (NowTime.wHour < 13))
			{
				string MemoAlarmReply = "现在时间是\n" + TimeRepo + "\n如果你在吃午饭的话就听着就好，你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 13) && (NowTime.wHour < 17))
			{
				string MemoAlarmReply = "喝过下午茶了么？现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 17) && (NowTime.wHour < 20))
			{
				string MemoAlarmReply = "现在时间是\n" + TimeRepo + "\n夜生活就要开始了呢。你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else //if ((NowTime.wHour >= 20) && (NowTime.wHour < 24))
			{
				string MemoAlarmReply = "现在时间是\n" + TimeRepo + "\n我么？我现在精神得很。你让我现在提醒你你的备忘录\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
		}
		MemoScan++;
	}
	return;
}

string MemoAlarmControl(long long LowerQQID, bool MemoAlarmSwitchOn)
{
	if (LowerQQID == MasterQQID)
	{
		if (MemoAlarmSwitchOn)
		{
			if (MemoScanOn)
			{
				return "定时提醒已经在进行了";
			}
			else
			{
				MemoScanOn = true;
				return "那么从现在开始定时提醒有需要的顾客吧";
			}
		}
		else
		{
			if (!MemoScanOn)
			{
				return "定时提醒早就取消了";
			}
			else
			{
				MemoScanOn = false;
				return "已取消定时提醒";
			}
		}
	}
	else
	{
		return "你不是我的老板，我才不听你的";
	}
}

string SetMemoAlarm(long long LowerQQID, long long GroupID, const int AlarmTime, bool IsPersional, bool AlarmOn)
{
	if (!MemoMap.count(LowerQQID))
		return "您还没在这里设置过备忘";
	MemoType TemMemo(GroupID, AlarmTime, IsPersional, AlarmOn);
	TemMemo = MemoMap[LowerQQID].Memo;
	MemoMap[LowerQQID] = TemMemo;
	if (AlarmOn)
		return "好的，我会在每天" + to_string(AlarmTime) + "点提醒你的";
	else
		return "备忘录的每日提醒已取消";
}

void MemoPack(bool isPack, bool ClearMemory)
{
	string strFileLoc = getAppDirectory();
	if (isPack)
	{
		ofstream ofstreamMemo(strFileLoc + "Memo.RDconf", ios::out | ios::trunc);
		for (auto it1 = MemoMap.begin(); it1 != MemoMap.end(); ++it1)
		{
			ofstreamMemo << "{memostart} " << it1->first << " " << it1->second.GroupID << " " << it1->second.AlarmTime << " " << it1->second.isPersonal << " " << it1->second.AlarmOn << std::endl;
			string Memo;
			for (auto it2 = it1->second.Memo.begin(); it2 != it1->second.Memo.end(); ++it2)
			{
				Memo = *it2;
				while (Memo.find(' ') != string::npos)Memo.replace(Memo.find(' '), 1, "{space}");
				while (Memo.find('\t') != string::npos)Memo.replace(Memo.find('\t'), 1, "{tab}");
				while (Memo.find('\n') != string::npos)Memo.replace(Memo.find('\n'), 1, "{endl}");
				while (Memo.find('\r') != string::npos)Memo.replace(Memo.find('\r'), 1, "{enter}");
				ofstreamMemo << Memo << std::endl;
			}
		}
		ofstreamMemo.close();
		if (ClearMemory)
			MemoMap.clear();
		return;
	}
	else
	{
		ifstream ifstreamMemo(strFileLoc + "Memo.RDconf");
		if (ifstreamMemo)
		{
			long long QQID, GroupID;
			unsigned int AlarmTime;
			bool isPersonal, AlarmOn;
			string strMemo;
			MemoType TemMemoType;
			while (ifstreamMemo >> strMemo)
			{
				while (strMemo.find("{space}") != string::npos)strMemo.replace(strMemo.find("{space}"), 7, " ");
				while (strMemo.find("{tab}") != string::npos)strMemo.replace(strMemo.find("{tab}"), 5, "\t");
				while (strMemo.find("{endl}") != string::npos)strMemo.replace(strMemo.find("{endl}"), 6, "\n");
				while (strMemo.find("{enter}") != string::npos)strMemo.replace(strMemo.find("{enter}"), 7, "\r");
				if (strMemo.find("{memostart}") != string::npos)
				{
					ifstreamMemo >> QQID >> GroupID >> AlarmTime >> isPersonal >> AlarmOn;
					TemMemoType = MemoType(GroupID, AlarmTime, isPersonal, AlarmOn);
					MemoMap[QQID] = TemMemoType;
					continue;
				}
				MemoRecorder(QQID, strMemo, MemoRecoEnum::Add, 0);
			}
			ifstreamMemo.close();
		}
		return;
	}
}