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
		return "“好!我记住啦！”多多潦草地在手里的羊皮纸上划拉几下";
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
		return "“好!我记住啦！”多多在手里的羊皮纸上加上了几道爪痕";
	}
	case MemoRecoEnum::Clear:
	{
		if (!MemoMap.count(LowerQQID))
			return "多多又看了看手里空白的羊皮纸：“你没有让我记下什么呀——”";
		if (ClearWhere)
		{
			MemoType TemMemo(MemoMap[LowerQQID]);
			if (TemMemo.Memo.size() < ClearWhere)
				return "“可是，可是你只记录了" + to_string(TemMemo.Memo.size()) + "条备忘呀！”多多无辜地戳戳手里的羊皮纸";
			TemMemo -= ClearWhere;
			if (TemMemo.Memo.empty())
				MemoMap.erase(LowerQQID);
			else
				MemoMap[LowerQQID] = TemMemo;
			return "“呲啦——”多多将羊皮纸划开了一道口子，“好！删掉！”";
		}
		else
		{
			MemoMap.erase(LowerQQID);
			return "备忘录全部删光光啦！";
		}
	}
	default:
		return "“好像出了什么问题……”";
	}
}

string MemoRecorderGet(long long LowerQQID)
{
	if (MemoMap.count(LowerQQID))
		return "在多多这儿记下了这样的东西——" + MemoMap[LowerQQID].Display();
	else
		return "喵……好像根本就什么都没有记下嘛！";
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
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多迷迷糊糊地打着呼噜，卷在尾巴上的一张羊皮纸展开在你脚边\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 5) && (NowTime.wHour < 8))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n带着新鲜的海风气味的多多将写着备忘的羊皮纸交给你，叼着一只刚捉住的鱼向你炫耀着\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 8) && (NowTime.wHour < 11))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多躺在甲板上懒洋洋地晒着太阳，一旁的备忘羊皮纸上写着这样的内容\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 11) && (NowTime.wHour < 13))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多上蹿下跳地闹着要吃肉，备忘录掉在你脚边，上面写着这样的内容\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 13) && (NowTime.wHour < 17))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多仰面瘫在瞭望台上呼呼大睡，尾巴尖卷着一张备忘羊皮纸\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else if ((NowTime.wHour >= 17) && (NowTime.wHour < 20))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多登上瞭望台看日落，一边有些恶趣味地大声读着这样的备忘\n" + MemoScan->second.Display();
				CQ::sendPrivateMsg(MemoScan->first, MemoAlarmReply);
				if (!MemoScan->second.isPersonal)
				{
					MemoAlarmReply = "[CQ:at,qq=" + to_string(MemoScan->first) + "]" + MemoAlarmReply;
					CQ::sendGroupMsg(MemoScan->second.GroupID, MemoAlarmReply);
				}
			}
			else //if ((NowTime.wHour >= 20) && (NowTime.wHour < 24))
			{
				string MemoAlarmReply = "现在是\n" + TimeRepo + "\n多多精神地四处乱窜，一张写着备忘的羊皮纸被抛在一旁，上面写着这样的内容\n" + MemoScan->second.Display();
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
				return "定时提醒已经启用了喵——";
			}
		}
		else
		{
			if (!MemoScanOn)
			{
				return "定时提醒早就取消了喵——";
			}
			else
			{
				MemoScanOn = false;
				return "定时提醒已经取消了喵——";
			}
		}
	}
	else
	{
		return "等什么时候你成了我的大副，或者打败我成为新的船长，再来对我发号施令吧！";
	}
}

string SetMemoAlarm(long long LowerQQID, long long GroupID, const int AlarmTime, bool IsPersional, bool AlarmOn)
{
	if (!MemoMap.count(LowerQQID))
		return "“你根本就没让我记过什么东西嘛！”多多捏着手里的空羊皮纸，“让我提醒你什么，喂我吃零食嘛？”";
	MemoType TemMemo(GroupID, AlarmTime, IsPersional, AlarmOn);
	TemMemo = MemoMap[LowerQQID].Memo;
	MemoMap[LowerQQID] = TemMemo;
	if (AlarmOn)
		return "好喔！多多会在每天" + to_string(AlarmTime) + "点提醒你的！";
	else
		return "多多把羊皮纸小心翼翼地揣进帽子里：“以后想看备忘了就主动来找我吧——”";
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