#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <cstdio>
#include <string>
#include "cqdefine.h"
#include "CQAPI_EX.h"
#include "MasterData.h"
#include "CDMainCirculate.h"
#include "Constchar_to_LPCWSTR.h"/*const char* -> LPCWSTR*/
#include <time.h>/*以下为计时模块*/
#include <Windows.h>
#include "LastMsgTimeRecorder.h"

#pragma comment( lib,"winmm.lib" )/*以上为计时模块*/

using namespace std;
using namespace CQ;

/*LTS开关的初始值*/
bool LMTScanOn = 0;
long long TimeDifferenceThreshold = 7 * 24 * 60 * 60,/*闲置退群时长阈值（秒）*/
	   	  SoonReportTime = 60 * 60 * 24;/*因闲置退群预警时间（秒，仅主人可见）*/
int RepoDayThreshold = 3,/*可查询的闲置时长阈值（天）*/
	ScanInterval = 60 * 60 * 1000,/*超时退群扫描间隔/毫秒*/
	OrderInterval = 60 * 1000;/*扫描停止命令的最大响应间隔/毫秒*/

map<long long, long long> LMTList;
set<long long> LMTWhiteList;

#if LMTLOG
void LMTLogRec(long long Target_ID, string Action)
{
	time_t SecNowtime;
	time(&SecNowtime);
	SYSTEMTIME FormatTime;
	GetLocalTime(&FormatTime);
	string strNowTime, TemStrTime;

	TemStrTime = to_string(FormatTime.wYear);
	while (TemStrTime.length() < 4)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "年";

	TemStrTime = to_string(FormatTime.wMonth);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "月";

	TemStrTime = to_string(FormatTime.wDay);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "日";

	TemStrTime = to_string(FormatTime.wHour);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "时";

	TemStrTime = to_string(FormatTime.wMinute);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "分";

	TemStrTime = to_string(FormatTime.wSecond);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "秒";

	TemStrTime = to_string(FormatTime.wMilliseconds);
	while (TemStrTime.length() < 3)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "毫秒" + "\t";

	TemStrTime = to_string(SecNowtime);
	while (TemStrTime.length() < 10)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "\t";

	TemStrTime = to_string(Target_ID);
	while (TemStrTime.length() < 10)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "\t";

	strNowTime = strNowTime + Action;

	ofstream LastTimeMsgLog(LastTimeMsgLogPos, ios::app);
	LastTimeMsgLog << strNowTime << endl;
	LastTimeMsgLog.close();
}
#endif // LMTLOG

void LastMsgTimeRecorder(long long Target_ID)/*可将该函数插入至所有有效命令回复下以记录时间*/
{
	if (!Target_ID || LMTWhiteList.count(Target_ID))
		return;
	time_t nowtime;
	time(&nowtime);
	LMTList[Target_ID] = nowtime;
	return;
}

string WhiteListControl(long long LowerQQID, long long Target_ID, bool isWriteIn)//有内置判断是否是master的功能
{
	if (LowerQQID == MasterQQID)
		if (isWriteIn)
			if (LMTWhiteList.count(Target_ID))
				return"这个群已经在闲置白名单上了！";
			else
			{
				map<long long, string> GroupList = CQ::getGroupList();
				if (!GroupList.count(Target_ID))
					return"您拨打的号码是空号！";
				LMTWhiteList.insert(Target_ID);
				if (LMTList.count(Target_ID))
					LMTList.erase(Target_ID);
				return"已将该群加入闲置白名单！";
			}
		else
			if (LMTWhiteList.count(Target_ID))
			{
				LMTWhiteList.erase(Target_ID);
				LastMsgTimeRecorder(Target_ID);
				return"已将该群从闲置白名单中移除！";
			}
			else
				return"该群不在闲置白名单上，无需移除！";
	else
		if (LMTList.count(Target_ID))
			return"这个群在闲置白名单上哦，我不会随便退群啦";
		else
			return"这个群不在闲置白名单上哦，或者这里不是群聊？";
}

LMTMsgType RefrList(long long LowerQQID)
{
	if (LowerQQID != MasterQQID)
		return LMTMsgType(false, "等什么时候你成了我的大副，或者打败我成为新的船长，再来对我发号施令吧！");
	bool isEffective = false;
	map<long long, string> GroupList = CQ::getGroupList();
	map<long long, long long>::iterator LMTRefr = LMTList.begin();/*闲置名单的迭代*/
	map<long long, string>::iterator GroupListRefr = GroupList.begin();/*所有群组列表的迭代*/
	set<long long>::iterator WhiteListRefr = LMTWhiteList.begin();/*白名单的迭代*/
	map<long long, long long> temMap;
	set<long long> temSet;
	string LMTListtoGroupList, GroupListtoLMTList, WhiteListtoGroupList, WhiteListtoLMTList, RefrRepo = "已刷新列表";
	while (LMTRefr != LMTList.end())/*逐一检查闲置名单上的群是否在群名单里；闲置名单->群名单*/
	{
		if (GroupList.count(LMTRefr->first))
			temMap[LMTRefr->first] = LMTRefr->second;
		else
			LMTListtoGroupList = LMTListtoGroupList + "\n" + to_string(LMTRefr->first);
		LMTRefr++;
	}
	if (!LMTListtoGroupList.empty())
	{
		isEffective = true;
		RefrRepo = RefrRepo + "\n闲置名单里的这些群是无效的：" + LMTListtoGroupList;
		LMTList = temMap;
		temMap.clear();
	}
	else
		RefrRepo += "\n闲置列表上的群我都在";
	while (GroupListRefr != GroupList.end())/*逐一检查群名单里的群是否都在闲置名单里；群名单->闲置名单*/
	{
		if (!LMTList.count(GroupListRefr->first) && !LMTWhiteList.count(GroupListRefr->first))
		{
			LastMsgTimeRecorder(GroupListRefr->first);
			GroupListtoLMTList = GroupListtoLMTList + "\n" + to_string(GroupListRefr->first) + GroupListRefr->second;
		}
		GroupListRefr++;
	}
	if (!GroupListtoLMTList.empty())
	{
		isEffective = true;
		RefrRepo = RefrRepo + "\n已将这些群添加到闲置名单上：" + GroupListtoLMTList;
	}
	else
		RefrRepo += "\n没有遗漏的群组";
	while (WhiteListRefr != LMTWhiteList.end())/*逐一检查白名单上的群是否有效以及是否在闲置名单里*/
	{
		if (GroupList.count(*WhiteListRefr))
			temSet.insert(*WhiteListRefr);
		else
			WhiteListtoGroupList = WhiteListtoGroupList + "\n" + to_string(*WhiteListRefr);
		if (LMTList.count(*WhiteListRefr))
		{
			LMTList.erase(*WhiteListRefr);
			WhiteListtoLMTList = WhiteListtoLMTList + "\n" + to_string(*WhiteListRefr);
		}
		WhiteListRefr++;
	}
	if (!WhiteListtoGroupList.empty())
	{
		isEffective = true;
		RefrRepo = RefrRepo + "\n这些白名单上的群是无效的" + WhiteListtoGroupList;
		LMTWhiteList = temSet;
	}
	else
		RefrRepo += "\n白名单上的群我全都在";
	temSet.clear();
	if (!WhiteListtoLMTList.empty())
	{
		isEffective = true;
		RefrRepo = RefrRepo + "\n已将这些群从闲置名单上删除" + WhiteListtoLMTList;
	}
	else
		RefrRepo += "闲置名单上没有无效的群";
	return LMTMsgType(isEffective, RefrRepo);
}

void LeaveGroupScan()
{
	LMTMsgType LeaveGroupMsg = RefrList(MasterQQID);
	if(LeaveGroupMsg.isEffective)
		CQ::sendPrivateMsg(MasterQQID, LeaveGroupMsg.LMTMsg);
	time_t Nowtime;
	time(&Nowtime);
	map<long long, string> GroupList = CQ::getGroupList();
	map<long long, string>::iterator GroupCount = GroupList.begin();
	while (GroupCount != GroupList.end())
	{
		if (LMTList.count(GroupCount->first))
		{
			if (Nowtime >= (LMTList[GroupCount->first] + TimeDifferenceThreshold))
			{
				string GroupLeaveReply = "由于已超过" + to_string(TimeDifferenceThreshold / (60 * 60 * 24)) + "天未在群" + to_string(GroupCount->first) + GroupCount->second + "中发言，已退出该群\n最后一次发言在：" + to_string(LMTList[GroupCount->first]) + "当时时间：" + to_string(Nowtime);
				CQ::sendPrivateMsg(MasterQQID, GroupLeaveReply);
				string GroupLeavePrompt = "由于超过" + to_string(TimeDifferenceThreshold / (60 * 60 * 24)) + "天未在贵群中回应掷骰指令，为了能让服务器的资源服务更多更需要的用户，多多将退出贵群\n本次退群将不会记录进黑名单，若仍然需要多多可再次邀请";
				CQ::sendGroupMsg(GroupCount->first, GroupLeavePrompt);
				if (getGroupMemberInfo(GroupCount->first, getLoginQQ()).permissions == 3)/*退群指令*/
					setGroupLeave(GroupCount->first, 1);
				else
					setGroupLeave(GroupCount->first, 0);
				LMTList.erase(GroupCount->first);
				Sleep(1000);/*消息发送最小间隔（毫秒）*/
				continue;
			}
			if (Nowtime >= (LMTList[GroupCount->first] + TimeDifferenceThreshold - SoonReportTime) && (Nowtime < (LMTList[GroupCount->first] + TimeDifferenceThreshold - SoonReportTime + (60 * 60))))
			{
				string SoonGroupLeaveRport = "由于被放置太久，我要在" + to_string(SoonReportTime / (60 * 60)) + "小时后退出群聊" + to_string(GroupCount->first) + GroupCount->second + "啦";
				CQ::sendPrivateMsg(MasterQQID, SoonGroupLeaveRport);
				Sleep(1000);/*消息发送最小间隔（毫秒）*/
			}
		}
		GroupCount++;
	}	
}

string LeaveGroupScanControl(bool SwitchMsg, long long LowerQQID)
{
	if (LowerQQID == MasterQQID)/*判断是否是master*/
		if (SwitchMsg)/*开启扫描命令*/
			if (LMTScanOn == SwitchMsg)
				return "被闲置监测处于启用状态喵——";
			else
			{
				LMTScanOn = 1;
				return "被闲置监测启用成功！";
			}
		else/*关闭扫描命令*/
			if (LMTScanOn == SwitchMsg)
				return "被闲置监测处于停用状态喵——";
			else
			{
				LMTScanOn = 0;
				return "被闲置监测停用成功！";
			}
	else
		return "等什么时候你成了我的大副，或者打败我成为新的船长，再来对我发号施令吧！";
}

string LMTRecReport(long long LowerQQID)
{
	if (LowerQQID != MasterQQID)
		return "等什么时候你成了我的大副，或者打败我成为新的船长，再来对我发号施令吧！";
	string ReportReply = "闲置白名单上的群有：";
	set<long long>::iterator WhiteListCount = LMTWhiteList.begin();
	map<long long, string> GroupList = CQ::getGroupList();
	map<long long, long long>::iterator LMTListCount = LMTList.begin();
	while (WhiteListCount != LMTWhiteList.end())
	{
		ReportReply = ReportReply + "\n" + GroupList[*WhiteListCount] + to_string(*WhiteListCount);
		WhiteListCount++;
	}
	time_t Nowtime;
	time(&Nowtime);
	ReportReply += "\n闲置时间超过" + to_string(RepoDayThreshold) + "天的群有：";
	while (LMTListCount != LMTList.end())
	{
		if ((Nowtime - LMTListCount->second) / (60 * 60 * 24) >= RepoDayThreshold)
			ReportReply = ReportReply + "\n" + GroupList[LMTListCount->first] + to_string(LMTListCount->first) + "  "
			+ to_string((Nowtime - LMTListCount->second) / (60 * 60 * 24)) + "天"
			+ to_string(((Nowtime - LMTListCount->second) % (60 * 60 * 24)) / (60 * 60)) + "小时";
		LMTListCount++;
	}
	return ReportReply;
}

void ListPack(bool isPack, bool ClearMemory)
{
	string strFileLoc = getAppDirectory();
	if (isPack)
	{
		ofstream ofstreamLMTList(strFileLoc + "LMTList.RDconf", ios::out | ios::trunc);
		for (auto it = LMTList.begin(); it != LMTList.end(); ++it)
		{
			ofstreamLMTList << it->first << " " << it->second << std::endl;
		}
		ofstreamLMTList.close();

		ofstream ofstreamLMTWhiteList(strFileLoc + "LMTWhiteList.RDconf", ios::out | ios::trunc);
		for (auto it = LMTWhiteList.begin(); it != LMTWhiteList.end(); ++it)
		{
			ofstreamLMTWhiteList << *it << std::endl;
		}
		ofstreamLMTWhiteList.close();

		if (ClearMemory)
		{
			LMTList.clear();
			LMTWhiteList.clear();
		}
	}
	else
	{
		ifstream ifstreamLMTList(strFileLoc + "LMTList.RDconf");
		if (ifstreamLMTList)
		{
			long long GroupID, LMTNum;
			while (ifstreamLMTList >> GroupID >> LMTNum)
				LMTList[GroupID] = LMTNum;
		}
		ifstreamLMTList.close();

		ifstream ifstreamLMTWhiteList(strFileLoc + "LMTWhiteList.RDconf");
		if (ifstreamLMTWhiteList)
		{
			long long GroupID;
			while (ifstreamLMTWhiteList >> GroupID)
			{
				LMTWhiteList.insert(GroupID);
			}
		}
		ifstreamLMTWhiteList.close();
	}
	return;
}