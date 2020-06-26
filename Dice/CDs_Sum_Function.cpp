#include <map>
#include <set>
#include <iostream>
#include <string>
#include <Windows.h>
#include <iostream>
#include <fstream>

#include "CDs_Sum_Function.h"
#include "CQAPI_EX.h"

using namespace std;
using namespace CQ;

map<long long, CheckSumStruct> CheckSumMap;

string CheckSumEnum_to_String(CheckSumEnum SuccessLevel)
{
	switch (SuccessLevel)
	{
	case CheckSumEnum::CriticalSuccess:return"大成功";
	case CheckSumEnum::ExttremeSuccess:return"极难成功";
	case CheckSumEnum::HardSuccess:return"困难成功";
	case CheckSumEnum::Success:return"普通成功";
	case CheckSumEnum::Failure:return"普通失败";
	case CheckSumEnum::Fumble:return"大失败";
	default:return"无效的成功等级";
	}
}

void CheckSumRecorder(long long QQ_ID, CheckSumEnum SuccessSum)
{
	SYSTEMTIME NowTime;
	GetLocalTime(&NowTime);
	if (CheckSumMap.count(QQ_ID))
	{
		CheckSumStruct TemSumStruct(CheckSumMap[QQ_ID]);
		TemSumStruct.CheckSum[SuccessSum][NowTime.wDayOfWeek]++;
		TemSumStruct.CheckSum[SuccessSum][7]++;
		CheckSumMap[QQ_ID] = TemSumStruct;
	}
	else
	{
		CheckSumStruct TemSumStruct;
		TemSumStruct.CheckSum[SuccessSum][NowTime.wDayOfWeek]++;
		TemSumStruct.CheckSum[SuccessSum][7]++;
		CheckSumMap[QQ_ID] = TemSumStruct;
	}
	return;
}

string CheckSumReporter(long long QQ_ID)
{
	CheckSumStruct TemSumStruct;
	if (CheckSumMap.count(QQ_ID))
		TemSumStruct = CheckSumMap[QQ_ID];
	string ReportReply;
	int WeekSuccess = 0, WeekFailure = 0, SumSuccess = 0, SumFailure = 0;
	for (CheckSumEnum i = CheckSumEnum::CriticalSuccess; i <= CheckSumEnum::Fumble; i = CheckSumEnum(i + 1))
	{
		int WeekSum = 0;
		for (int j = 0; j <= 5; j++)
		{
			if (i <= CheckSumEnum::Success)
				WeekSuccess += TemSumStruct.CheckSum[i][j];
			else
				WeekFailure += TemSumStruct.CheckSum[i][j];
			WeekSum += TemSumStruct.CheckSum[i][j];
		}
		if (i <= CheckSumEnum::Success)
			SumSuccess += TemSumStruct.CheckSum[i][7];
		else
			SumFailure += TemSumStruct.CheckSum[i][7];
		ReportReply = ReportReply + "\n" + CheckSumEnum_to_String(i) + "\t" + to_string(TemSumStruct.CheckSum[i][7]) + "（" + to_string(WeekSum) + "）";
	}
	ReportReply = ReportReply + "\n总计：" + to_string(SumSuccess) + "（" + to_string(WeekSuccess) + "）次成功" + to_string(SumFailure) + "（" + to_string(WeekFailure) + "）次失败";
	return ReportReply;
}

void CheckSumPack(bool isPack,bool ClearMemory)
{
	string strFileLoc = getAppDirectory();
	if (isPack)
	{
		ofstream ofstreamCheckSum(strFileLoc + "CheckSum.RDconf", ios::out | ios::trunc);
		for (auto it = CheckSumMap.begin(); it != CheckSumMap.end(); ++it)
		{
			ofstreamCheckSum << it->first;
			for (CheckSumEnum i = CheckSumEnum::CriticalSuccess; i <= CheckSumEnum::Fumble; i = CheckSumEnum(i + 1))
			{
				for (int j = 0; j <= 7; j++)
				{
					ofstreamCheckSum << " " << it->second.CheckSum[i][j];
				}
			}
			ofstreamCheckSum << std::endl;
		}
		ofstreamCheckSum.close();
		if (ClearMemory)
			CheckSumMap.clear();
	}
	else
	{
		ifstream ofstreamCheckSum(strFileLoc + "CheckSum.RDconf");
		if (ofstreamCheckSum)
		{
			long long QQ_ID;
			int TemCheckSum[6][8];
			while (ofstreamCheckSum >> QQ_ID)
			{
				for (CheckSumEnum i = CheckSumEnum::CriticalSuccess; i <= CheckSumEnum::Fumble; i = CheckSumEnum(i + 1))
				{
					for (int j = 0; j <= 7; j++)
					{
						ofstreamCheckSum >> TemCheckSum[i][j];
					}
				}
				CheckSumMap[QQ_ID] = CheckSumStruct(TemCheckSum);
			}
		}
		ofstreamCheckSum.close();
	}
}

void CheckSumDailyRefr(SYSTEMTIME Time)
{
	map<long long, CheckSumStruct>::iterator CheckSumMapCount = CheckSumMap.begin();
	while (CheckSumMapCount != CheckSumMap.end())
	{
		CheckSumStruct TemSumStruct(CheckSumMapCount->second);
		for (CheckSumEnum i = CheckSumEnum::CriticalSuccess; i <= CheckSumEnum::Fumble;i = CheckSumEnum(i + 1))
		{
			TemSumStruct.CheckSum[i][Time.wDayOfWeek] = 0;
		}
		CheckSumMap[CheckSumMapCount->first] = TemSumStruct;
		CheckSumMapCount++;
	}
	return;
}