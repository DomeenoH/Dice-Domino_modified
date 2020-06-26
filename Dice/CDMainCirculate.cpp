#include <Windows.h>
#include "LastMsgTimeRecorder.h"
#include "MemoRecorder.h"
#include "TemClean.h"
#include "CDMainCirculate.h"
#include "MasterData.h"
#include "CQAPI_EX.h"
#include "CDs_Sum_Function.h"

/*总循环开关*/
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
	CQ::sendPrivateMsg(MasterQQID, "当前群列表里有" + to_string(GroupList.size()) + "个群,10s后将加载闲置监视列表,请核对正确后再启动LMT哟！");
	Sleep(10*1000);/*等待10s*/
	CheckSumPack(false, false);
	MemoPack(false, false);
	ListPack(false, false);
	CQ::sendPrivateMsg(MasterQQID, "闲置监视列表加载完成！目前列表里有" + to_string(LMTList.size()) + "个群，白名单里有" + to_string(LMTWhiteList.size()) + "个群，之后将每分钟备份一次");
	while (MainCirculateOn)
	{
		if (!MsgSend)
		{
			CQ::sendPrivateMsg(MasterQQID, "主循环已启动");
			MsgSend = true;
		}
		SYSTEMTIME NowTime;
		GetLocalTime(&NowTime);
		if (NowTime.wMinute != LastTime.wMinute)/*每分钟一次的事件*/
		{
			ListPack(true, false);
			CheckSumPack(true, false);
			LastTime.wMinute = NowTime.wMinute;
		}
		if (NowTime.wHour != LastTime.wHour)/*每小时一次的事件*/
		{
			if (MemoScanOn)
				MemoAlarmScan(NowTime);
			if(LMTScanOn)
				LeaveGroupScan();
			MemoPack(true, false);
			LastTime.wHour = NowTime.wHour;
		}
		if (NowTime.wDay != LastTime.wDay)/*每天执行一次的事件*/
		{
			if (!NowTime.wHour)/*仅在0点执行（防止重启导致的运行*/
			{
				CheckSumDailyRefr(NowTime);
			}
			isNewWeek = true;
			TemCQImgClean();
			LastTime.wDay = NowTime.wDay;
			GroupTestCount.clear();
		}
		if (NowTime.wDayOfWeek == 1 && isNewWeek)/*每周一执行一次的事件*/
		{
			isNewWeek = false;
		}
		if (NowTime.wMonth != LastTime.wMonth)/*每月执行一次的事件*/
		{
			LastTime.wMonth = NowTime.wMonth;
		}
		Sleep(60 * 1000);/*等待1分钟*/
	}
	return 1;
}