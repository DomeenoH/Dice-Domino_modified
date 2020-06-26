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
		return "682甩了甩尾巴，姑且算是在表示记住了";
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
		return "好!我记住了嗷！”盐酸池里冒出了一串泡泡";
	}
	case MemoRecoEnum::Clear:
	{
		if (!MemoMap.count(LowerQQID))
			return "“你没有让我记下什么呀——”";
		if (ClearWhere)
		{
			MemoType TemMemo(MemoMap[LowerQQID]);
			if (TemMemo.Memo.size() < ClearWhere)
				return "“你只记录了" + to_string(TemMemo.Memo.size()) + "条备忘嗷！”";
			TemMemo -= ClearWhere;
			if (TemMemo.Memo.empty())
				MemoMap.erase(LowerQQID);
			else
				MemoMap[LowerQQID] = TemMemo;
			return "“本大爷把这条备忘忘记了嗷！”";
		}
		else
		{
			MemoMap.erase(LowerQQID);
			return "备忘录全部删光光了嗷！";
		}
	}
	default:
		return "“好像出了什么问题……”";
	}
}

string MemoRecorderGet(long long LowerQQID)
{
	if (MemoMap.count(LowerQQID))
		return "在682这儿记下了这样的东西——" + MemoMap[LowerQQID].Display();
	else
		return "……诶嘿，好像你没有让本大爷记下什么。";
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
				string MemoAlarmReply = "现在是……\n" + TimeRepo + "\n调查员快清醒一点！！你让我现在提醒你你的笔记！\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 5) && (NowTime.wHour < 8))
			{
				string MemoAlarmReply = "早嗷嗷！！现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的笔记\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 8) && (NowTime.wHour < 11))
			{
				string MemoAlarmReply = "上午好嗷！现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的笔记\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 11) && (NowTime.wHour < 13))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n你什么时候来喂本大爷啊！！你让我现在提醒你你的笔记\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 13) && (NowTime.wHour < 17))
			{
				string MemoAlarmReply = "肚子有点饿啊……现在时间是\n" + TimeRepo + "\n你让我现在提醒你你的笔记\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 17) && (NowTime.wHour < 20))
			{
				string MemoAlarmReply = "现在时间是\n" + TimeRepo + "\n下班前别忘了你的笔记——\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else //if ((NowTime.wHour >= 20) && (NowTime.wHour < 24))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n我么？本大爷不需要睡觉！你让我现在提醒你你的笔记\n" + MemoScan->second.Display();
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
				return "定时提醒已经启用了嗷——";
			}
		}
		else
		{
			if (!MemoScanOn)
			{
				return "定时提醒早就取消了嗷——";
			}
			else
			{
				MemoScanOn = false;
				return "定时提醒已经取消了嗷——";
			}
		}
	}
	else
	{
		return "是研究员多米诺嘛？有个D级想要偷用你的权限啊！！";
	}
}

string SetMemoAlarm(long long LowerQQID, long long GroupID, const int AlarmTime, bool IsPersional, bool AlarmOn)
{
	if (!MemoMap.count(LowerQQID))
		return "“你根本就没让我记过什么东西嘛！”";
	MemoType TemMemo(GroupID, AlarmTime, IsPersional, AlarmOn);
	TemMemo = MemoMap[LowerQQID].Memo;
	MemoMap[LowerQQID] = TemMemo;
	if (AlarmOn)
		return "好喔！我会在每天" + to_string(AlarmTime) + "点提醒你的！";
	else
		return "“那本大爷以后就不提醒你了嗷！以后想看备忘了就主动来找我吧——”";
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