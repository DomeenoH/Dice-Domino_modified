#pragma once


using namespace std;

#ifndef Memo_Recorder
#define Memo_Recorder

#include <string>
#include <iostream>   
#include <windows.h>
#include <list>

extern bool MemoScanOn;
struct MemoType
{
	MemoType(long long a = 0, unsigned int b = 0, bool c = false, bool d = false) :GroupID(a), AlarmTime(b), isPersonal(c), AlarmOn(d)/*ȫ�ճ�ʼ��*/
	{
	}
	MemoType(string e, long long a = 0, unsigned int b = 0, bool c = false, bool d = false) :GroupID(a), AlarmTime(b), isPersonal(c), AlarmOn(d)/*���е�һ�������ĳ�ʼ��*/
	{
		Memo.push_back(e);
	}
	list<string> Memo;
	long long GroupID;
	unsigned int AlarmTime;
	bool isPersonal, AlarmOn;
	string Display()/*չʾ����¼��������*/
	{
		string Reply;
		list<string>::iterator MemoCount = Memo.begin();
		while (MemoCount != Memo.end())
		{
			Reply += "\n";
			Reply += *MemoCount;
			MemoCount++;
		}
		return Reply;
	}
	MemoType operator+=(string a)/*/���һ������¼*/
	{
		Memo.push_back(a);
		return *this;
	}
	MemoType operator=(list<string> a)/*��ʼ�����α���¼*/
	{
		Memo = a;
		return *this;
	}
	MemoType operator+=(list<string> a)/*������α���¼*/
	{
		Memo.splice(Memo.end(), a);
		return *this;
	}
	MemoType operator-=(unsigned int a)/*ɾ��һ������¼*/
	{
		if (Memo.size() < a)
			return *this;
		list<string>::iterator MemoCount = Memo.begin();
		for (int i = 1; i != a; i++)
		{
			MemoCount++;
		}
		Memo.erase(MemoCount);
		return *this;
	}
};
extern map<long long, MemoType> MemoMap;
enum class MemoRecoEnum :int
{
	New = 1,
	Add = 2,
	Clear = 3
};
string MemoRecorder(long long LowerQQID, string MemoIn, MemoRecoEnum EnumSwitch, unsigned int ClearWhere);
string MemoRecorderGet(long long LowerQQID);
void MemoAlarmScan(SYSTEMTIME NowTime);
string MemoAlarmControl(long long LowerQQID, bool MemoAlarmSwitchOn);
string SetMemoAlarm(long long LowerQQID, long long GroupID, const int AlarmTime, bool IsPersional, bool AlarmOn);
void MemoPack(bool isPack, bool ClearMemory);

#endif // !Memo_Recorder
