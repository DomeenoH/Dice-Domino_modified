#include <Windows.h>
#include "LastMsgTimeRecorder.h"
#include "MemoRecorder.h"
#include "TemClean.h"
#include "CDMainCirculate.h"
#include "MasterData.h"
#include "CQAPI_EX.h"
#include "CDs_Sum_Function.h"

/*��ѭ������*/
bool MainCirculateOn = 1;
bool MsgSend = false;

DWORD WINAPI MainCirculate(LPVOID lpParamter)
{
	bool isNewWeek = true;
	SYSTEMTIME LastTime;
	GetLocalTime(&LastTime);

	LastTime.wMinute--;
	LastTime.wHour--;
	LastTime.wDay--;
	LastTime.wMonth--;

	map<long long, string> GroupList = CQ::getGroupList();
	CQ::sendPrivateMsg(MasterQQID, "��ǰȺ�б�����" + to_string(GroupList.size()) + "��Ⱥ,10s�󽫼������ü����б�,��˶���ȷ��������LMTӴ��");
	Sleep(10*1000);/*�ȴ�10s*/
	CheckSumPack(false, false);
	MemoPack(false, false);
	ListPack(false, false);
	CQ::sendPrivateMsg(MasterQQID, "���ü����б������ɣ�Ŀǰ�б�����" + to_string(LMTList.size()) + "��Ⱥ������������" + to_string(LMTWhiteList.size()) + "��Ⱥ��֮��ÿ���ӱ���һ��");
	while (MainCirculateOn)
	{
		if (!MsgSend)
		{
			CQ::sendPrivateMsg(MasterQQID, "��ѭ��������");
			MsgSend = true;
		}
		SYSTEMTIME NowTime;
		GetLocalTime(&NowTime);
		if (NowTime.wMinute != LastTime.wMinute)/*ÿ����һ�ε��¼�*/
		{
			ListPack(true, false);
			CheckSumPack(true, false);
			LastTime.wMinute = NowTime.wMinute;
		}
		if (NowTime.wHour != LastTime.wHour)/*ÿСʱһ�ε��¼�*/
		{
			if (MemoScanOn)
				MemoAlarmScan(NowTime);
			if(LMTScanOn)
				LeaveGroupScan();
			MemoPack(true, false);
			LastTime.wHour = NowTime.wHour;
		}
		if (NowTime.wDay != LastTime.wDay)/*ÿ��ִ��һ�ε��¼�*/
		{
			if (!NowTime.wHour)/*����0��ִ�У���ֹ�������µ�����*/
			{
				CheckSumDailyRefr(NowTime);
			}
			isNewWeek = true;
			TemCQImgClean();
			LastTime.wDay = NowTime.wDay;
			GroupTestCount.clear();
		}
		if (NowTime.wDayOfWeek == 1 && isNewWeek)/*ÿ��һִ��һ�ε��¼�*/
		{
			isNewWeek = false;
		}
		if (NowTime.wMonth != LastTime.wMonth)/*ÿ��ִ��һ�ε��¼�*/
		{
			LastTime.wMonth = NowTime.wMonth;
		}
		Sleep(60 * 1000);/*�ȴ�1����*/
	}
	return 1;
}