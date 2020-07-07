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
#include <time.h>/*����Ϊ��ʱģ��*/
#include <Windows.h>
#include "LastMsgTimeRecorder.h"

#pragma comment( lib,"winmm.lib" )/*����Ϊ��ʱģ��*/

using namespace std;
using namespace CQ;

/*LTS���صĳ�ʼֵ*/
bool LMTScanOn = 0;
long long TimeDifferenceThreshold = 7 * 24 * 60 * 60,/*������Ⱥʱ����ֵ���룩*/
	   	  SoonReportTime = 60 * 60 * 24;/*��������ȺԤ��ʱ�䣨�룬�����˿ɼ���*/
int RepoDayThreshold = 3,/*�ɲ�ѯ������ʱ����ֵ���죩*/
	ScanInterval = 60 * 60 * 1000,/*��ʱ��Ⱥɨ����/����*/
	OrderInterval = 60 * 1000;/*ɨ��ֹͣ����������Ӧ���/����*/

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
	strNowTime = strNowTime + TemStrTime + "��";

	TemStrTime = to_string(FormatTime.wMonth);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "��";

	TemStrTime = to_string(FormatTime.wDay);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "��";

	TemStrTime = to_string(FormatTime.wHour);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "ʱ";

	TemStrTime = to_string(FormatTime.wMinute);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "��";

	TemStrTime = to_string(FormatTime.wSecond);
	while (TemStrTime.length() < 2)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "��";

	TemStrTime = to_string(FormatTime.wMilliseconds);
	while (TemStrTime.length() < 3)
		TemStrTime = " " + TemStrTime;
	strNowTime = strNowTime + TemStrTime + "����" + "\t";

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

void LastMsgTimeRecorder(long long Target_ID)/*�ɽ��ú���������������Ч����ظ����Լ�¼ʱ��*/
{
	if (!Target_ID || LMTWhiteList.count(Target_ID))
		return;
	time_t nowtime;
	time(&nowtime);
	LMTList[Target_ID] = nowtime;
	return;
}

string WhiteListControl(long long LowerQQID, long long Target_ID, bool isWriteIn)//�������ж��Ƿ���master�Ĺ���
{
	if (LowerQQID == MasterQQID)
		if (isWriteIn)
			if (LMTWhiteList.count(Target_ID))
				return"���Ⱥ�Ѿ������ð��������ˣ�";
			else
			{
				map<long long, string> GroupList = CQ::getGroupList();
				if (!GroupList.count(Target_ID))
					return"������ĺ����ǿպţ�";
				LMTWhiteList.insert(Target_ID);
				if (LMTList.count(Target_ID))
					LMTList.erase(Target_ID);
				return"�ѽ���Ⱥ�������ð�������";
			}
		else
			if (LMTWhiteList.count(Target_ID))
			{
				LMTWhiteList.erase(Target_ID);
				LastMsgTimeRecorder(Target_ID);
				return"�ѽ���Ⱥ�����ð��������Ƴ���";
			}
			else
				return"��Ⱥ�������ð������ϣ������Ƴ���";
	else
		if (LMTList.count(Target_ID))
			return"���Ⱥ�����ð�������Ŷ���Ҳ��������Ⱥ��";
		else
			return"���Ⱥ�������ð�������Ŷ���������ﲻ��Ⱥ�ģ�";
}

LMTMsgType RefrList(long long LowerQQID)
{
	if (LowerQQID != MasterQQID)
		return LMTMsgType(false, "��ʲôʱ��������ҵĴ󸱣����ߴ���ҳ�Ϊ�µĴ������������ҷ���ʩ��ɣ�");
	bool isEffective = false;
	map<long long, string> GroupList = CQ::getGroupList();
	map<long long, long long>::iterator LMTRefr = LMTList.begin();/*���������ĵ���*/
	map<long long, string>::iterator GroupListRefr = GroupList.begin();/*����Ⱥ���б�ĵ���*/
	set<long long>::iterator WhiteListRefr = LMTWhiteList.begin();/*�������ĵ���*/
	map<long long, long long> temMap;
	set<long long> temSet;
	string LMTListtoGroupList, GroupListtoLMTList, WhiteListtoGroupList, WhiteListtoLMTList, RefrRepo = "��ˢ���б�";
	while (LMTRefr != LMTList.end())/*��һ������������ϵ�Ⱥ�Ƿ���Ⱥ�������������->Ⱥ����*/
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
		RefrRepo = RefrRepo + "\n�������������ЩȺ����Ч�ģ�" + LMTListtoGroupList;
		LMTList = temMap;
		temMap.clear();
	}
	else
		RefrRepo += "\n�����б��ϵ�Ⱥ�Ҷ���";
	while (GroupListRefr != GroupList.end())/*��һ���Ⱥ�������Ⱥ�Ƿ������������Ⱥ����->��������*/
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
		RefrRepo = RefrRepo + "\n�ѽ���ЩȺ��ӵ����������ϣ�" + GroupListtoLMTList;
	}
	else
		RefrRepo += "\nû����©��Ⱥ��";
	while (WhiteListRefr != LMTWhiteList.end())/*��һ���������ϵ�Ⱥ�Ƿ���Ч�Լ��Ƿ�������������*/
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
		RefrRepo = RefrRepo + "\n��Щ�������ϵ�Ⱥ����Ч��" + WhiteListtoGroupList;
		LMTWhiteList = temSet;
	}
	else
		RefrRepo += "\n�������ϵ�Ⱥ��ȫ����";
	temSet.clear();
	if (!WhiteListtoLMTList.empty())
	{
		isEffective = true;
		RefrRepo = RefrRepo + "\n�ѽ���ЩȺ������������ɾ��" + WhiteListtoLMTList;
	}
	else
		RefrRepo += "����������û����Ч��Ⱥ";
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
				string GroupLeaveReply = "�����ѳ���" + to_string(TimeDifferenceThreshold / (60 * 60 * 24)) + "��δ��Ⱥ" + to_string(GroupCount->first) + GroupCount->second + "�з��ԣ����˳���Ⱥ\n���һ�η����ڣ�" + to_string(LMTList[GroupCount->first]) + "��ʱʱ�䣺" + to_string(Nowtime);
				CQ::sendPrivateMsg(MasterQQID, GroupLeaveReply);
				string GroupLeavePrompt = "���ڳ���" + to_string(TimeDifferenceThreshold / (60 * 60 * 24)) + "��δ�ڹ�Ⱥ�л�Ӧ����ָ�Ϊ�����÷���������Դ����������Ҫ���û�����ཫ�˳���Ⱥ\n������Ⱥ�������¼��������������Ȼ��Ҫ�����ٴ�����";
				CQ::sendGroupMsg(GroupCount->first, GroupLeavePrompt);
				if (getGroupMemberInfo(GroupCount->first, getLoginQQ()).permissions == 3)/*��Ⱥָ��*/
					setGroupLeave(GroupCount->first, 1);
				else
					setGroupLeave(GroupCount->first, 0);
				LMTList.erase(GroupCount->first);
				Sleep(1000);/*��Ϣ������С��������룩*/
				continue;
			}
			if (Nowtime >= (LMTList[GroupCount->first] + TimeDifferenceThreshold - SoonReportTime) && (Nowtime < (LMTList[GroupCount->first] + TimeDifferenceThreshold - SoonReportTime + (60 * 60))))
			{
				string SoonGroupLeaveRport = "���ڱ�����̫�ã���Ҫ��" + to_string(SoonReportTime / (60 * 60)) + "Сʱ���˳�Ⱥ��" + to_string(GroupCount->first) + GroupCount->second + "��";
				CQ::sendPrivateMsg(MasterQQID, SoonGroupLeaveRport);
				Sleep(1000);/*��Ϣ������С��������룩*/
			}
		}
		GroupCount++;
	}	
}

string LeaveGroupScanControl(bool SwitchMsg, long long LowerQQID)
{
	if (LowerQQID == MasterQQID)/*�ж��Ƿ���master*/
		if (SwitchMsg)/*����ɨ������*/
			if (LMTScanOn == SwitchMsg)
				return "�����ü�⴦������״̬������";
			else
			{
				LMTScanOn = 1;
				return "�����ü�����óɹ���";
			}
		else/*�ر�ɨ������*/
			if (LMTScanOn == SwitchMsg)
				return "�����ü�⴦��ͣ��״̬������";
			else
			{
				LMTScanOn = 0;
				return "�����ü��ͣ�óɹ���";
			}
	else
		return "��ʲôʱ��������ҵĴ󸱣����ߴ���ҳ�Ϊ�µĴ������������ҷ���ʩ��ɣ�";
}

string LMTRecReport(long long LowerQQID)
{
	if (LowerQQID != MasterQQID)
		return "��ʲôʱ��������ҵĴ󸱣����ߴ���ҳ�Ϊ�µĴ������������ҷ���ʩ��ɣ�";
	string ReportReply = "���ð������ϵ�Ⱥ�У�";
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
	ReportReply += "\n����ʱ�䳬��" + to_string(RepoDayThreshold) + "���Ⱥ�У�";
	while (LMTListCount != LMTList.end())
	{
		if ((Nowtime - LMTListCount->second) / (60 * 60 * 24) >= RepoDayThreshold)
			ReportReply = ReportReply + "\n" + GroupList[LMTListCount->first] + to_string(LMTListCount->first) + "  "
			+ to_string((Nowtime - LMTListCount->second) / (60 * 60 * 24)) + "��"
			+ to_string(((Nowtime - LMTListCount->second) % (60 * 60 * 24)) / (60 * 60)) + "Сʱ";
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