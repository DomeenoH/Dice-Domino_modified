#pragma once

#ifndef CDCheckSum
#define CDCheckSum

#include <map>
#include <set>
#include <string>
#include <Windows.h>

enum CheckSumEnum :int
{
	CriticalSuccess = 0,
	ExttremeSuccess = 1,
	HardSuccess = 2,
	Success = 3,
	Failure = 4,
	Fumble = 5,
	SuccessLevelError = 6
};/*重载++失败了*/
struct CheckSumStruct
{
	CheckSumStruct()/*默认值*/
	{
		for (int i = 0; i <= 5; i++)
		{
			for (int j = 0; j <= 7; j++)
			{
				CheckSum[i][j] = 0;
			}
		}
	}
	CheckSumStruct(int a[6][8])/*允许使用数组初始化*/
	{
		for (int i = 0; i <= 5; i++)
		{
			for (int j = 0; j <= 7; j++)
			{
				CheckSum[i][j] = a[i][j];
			}
		}
	}
	int CheckSum[6][8];/*第一项是成功等级，第二项是星期(周日是0，总数是7）*/
	CheckSumStruct operator+=(CheckSumStruct a) const
	{
		for (int i = 0; i <= 5; i++)
		{
			for (int j = 0; j <= 7; j++)
			{
				a.CheckSum[i][j] += CheckSum[i][j];
			}
		}
		return a;
	}
};
extern std::map<long long, CheckSumStruct> CheckSumMap;
std::string CheckSumEnum_to_String(CheckSumEnum SuccessLevel);
void CheckSumRecorder(long long QQ_ID, CheckSumEnum SuccessSum);
std::string CheckSumReporter(long long QQ_ID);
void CheckSumPack(bool isPack, bool ClearMemory);
void CheckSumDailyRefr(SYSTEMTIME Time);

#endif//!CDCheckSum