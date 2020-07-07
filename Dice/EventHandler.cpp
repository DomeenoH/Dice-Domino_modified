#include "eventHandler.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <thread>
#include <chrono>
#include <mutex>

#include "APPINFO.h"
#include "RandomGenerator.h"
#include "RD.h"
#include "CQEVE_ALL.h"
#include "InitList.h"
#include "GlobalVar.h"
#include "NameStorage.h"
#include "GetRule.h"
#include "DiceMsgSend.h"
#include "CustomMsg.h"
#include "NameGenerator.h"
#include "MsgFormat.h"
#include "DiceNetwork.h"
#include "eventHandler.h"
#include "RDConstant.h"

#include "MasterData.h"
#include "Constchar_to_LPCWSTR.h"/*const char -> LPSWSTR*/
#include "CDMainCirculate.h"/*总循环控制*/
#include "LastMsgTimeRecorder.h"/*超时离群检测*/
#include "MemoRecorder.h"/*备忘录*/
#include "RandomImage.h"/*随机图片*/
#include "CDs_Sum_Function.h"/*总结*/
#include "CDs_UpertoLower.h"
#include "SpecialFunctionMap.h"

/*
TODO:
1. en可变成长检定
2. st多人物卡
3. st人物卡绑定
4. st属性展示，全属性展示以及排序
5. help优化
*/

using namespace std;
using namespace CQ;

unique_ptr<NameStorage> Name;

map<long long, int> DefaultDice;
map<long long, string> WelcomeMsg;
set<long long> DisabledGroup;
set<long long> DisabledDiscuss;
set<long long> DisabledJRRPGroup;
set<long long> DisabledJRRPDiscuss;
set<long long> DisabledMEGroup;
set<long long> DisabledHELPGroup;
set<long long> DisabledHELPDiscuss;
set<long long> DisabledOBGroup;
set<long long> DisabledOBDiscuss;
unique_ptr<Initlist> ilInitList;

struct SourceType
{
	SourceType(long long a, int b, long long c) : QQ(a), Type(b), GrouporDiscussID(c)
	{
	}
	SourceType(long long a, Dice::MsgType b, long long c) : QQ(a), Type(static_cast<int>(b)), GrouporDiscussID(c)
	{
	}
	long long QQ = 0;
	int Type = 0;
	long long GrouporDiscussID = 0;

	bool operator<(SourceType b) const
	{
		return this->QQ < b.QQ;
	}
};

using PropType = map<string, int>;
map<SourceType, PropType> CharacterProp;
multimap<long long, long long> ObserveGroup;
multimap<long long, long long> ObserveDiscuss;
string strFileLoc;



std::string strip(std::string origin)
{
	bool flag = true;
	while (flag)
	{
		flag = false;
		if (origin[0] == '!' || origin[0] == '.')
		{
			origin.erase(origin.begin());
			flag = true;
		}
		else if (origin.substr(0, 2) == "！" || origin.substr(0, 2) == "。")
		{
			origin.erase(origin.begin());
			origin.erase(origin.begin());
			flag = true;
		}
	}
	return origin;
}

std::string getName(long long QQ, long long GroupID = 0)
{
	// 群或讨论组
	if (GroupID)
	{
		// 群内昵称
		if (!strip(Name->get(GroupID, QQ)).empty())
			return strip(Name->get(GroupID, QQ));
		// 私聊全局昵称
		if (!strip(Name->get(0LL, QQ)).empty())
			return strip(Name->get(0LL, QQ));
		// 群名片-讨论组中会返回空字符串
		if (!getGroupMemberInfo(GroupID, QQ).GroupNick.empty())
			return strip(getGroupMemberInfo(GroupID, QQ).GroupNick);
		// 昵称
		return strip(getStrangerInfo(QQ).nick);
	}
	// 私聊
	// 私聊全局昵称
	if (!strip(Name->get(0LL, QQ)).empty())
		return strip(Name->get(0LL, QQ));
	// 昵称
	return strip(getStrangerInfo(QQ).nick);
}



#ifdef DEBUG
struct GrSourceType
{
	GrSourceType(long long a, int b, long long c) : QQ(a), Type(b), GrouporDiscussID(c)
	{
	}
	GrSourceType(long long a, Dice::MsgType b, long long c) : QQ(a), Type(static_cast<int>(b)), GrouporDiscussID(c)
	{
	}
	long long QQ = 0;
	int Type = 0;
	long long GrouporDiscussID = 0;

	bool operator<(GrSourceType b) const
	{
		if (b.GrouporDiscussID == 0)/*此时可按照QQID查询*/
			return this->QQ < b.QQ;
		else
			return this->QQ < b.QQ || this->GrouporDiscussID < b.GrouporDiscussID;
	}
};
#endif // DEBUG

/*map<GrSourceType, string> GroupCardList;   群卡绑定，QQID+群号->角色*/

namespace Dice
{
	void EventHandler::HandleEnableEvent()
	{
		//Wait until the thread terminates
		while (msgSendThreadRunning)
			Sleep(10);

		thread msgSendThread(SendMsg);
		msgSendThread.detach();
		strFileLoc = getAppDirectory();
		Sleep(10);
		HANDLE hThread = CreateThread(NULL, 0, MainCirculate, NULL, 0, NULL);/*mark总循环开启*/
		/*
		* 名称存储-创建与读取
		*/
		Name = make_unique<NameStorage>(strFileLoc + "Name.dicedb");
		ifstream ifstreamCharacterProp(strFileLoc + "CharacterProp.RDconf");
		if (ifstreamCharacterProp)
		{
			long long QQ, GrouporDiscussID;
			int Type, Value;
			string SkillName;
			while (ifstreamCharacterProp >> QQ >> Type >> GrouporDiscussID >> SkillName >> Value)
			{
				CharacterProp[SourceType(QQ, Type, GrouporDiscussID)][SkillName] = Value;
			}
		}
		ifstreamCharacterProp.close();

		ifstream ifstreamDisabledGroup(strFileLoc + "DisabledGroup.RDconf");
		if (ifstreamDisabledGroup)
		{
			long long Group;
			while (ifstreamDisabledGroup >> Group)
			{
				DisabledGroup.insert(Group);
			}
		}
		ifstreamDisabledGroup.close();
		ifstream ifstreamDisabledDiscuss(strFileLoc + "DisabledDiscuss.RDconf");
		if (ifstreamDisabledDiscuss)
		{
			long long Discuss;
			while (ifstreamDisabledDiscuss >> Discuss)
			{
				DisabledDiscuss.insert(Discuss);
			}
		}
		ifstreamDisabledDiscuss.close();
		ifstream ifstreamDisabledJRRPGroup(strFileLoc + "DisabledJRRPGroup.RDconf");
		if (ifstreamDisabledJRRPGroup)
		{
			long long Group;
			while (ifstreamDisabledJRRPGroup >> Group)
			{
				DisabledJRRPGroup.insert(Group);
			}
		}
		ifstreamDisabledJRRPGroup.close();
		ifstream ifstreamDisabledJRRPDiscuss(strFileLoc + "DisabledJRRPDiscuss.RDconf");
		if (ifstreamDisabledJRRPDiscuss)
		{
			long long Discuss;
			while (ifstreamDisabledJRRPDiscuss >> Discuss)
			{
				DisabledJRRPDiscuss.insert(Discuss);
			}
		}
		ifstreamDisabledJRRPDiscuss.close();
		ifstream ifstreamDisabledHELPGroup(strFileLoc + "DisabledHELPGroup.RDconf");
		if (ifstreamDisabledHELPGroup)
		{
			long long Group;
			while (ifstreamDisabledHELPGroup >> Group)
			{
				DisabledHELPGroup.insert(Group);
			}
		}
		ifstreamDisabledHELPGroup.close();
		ifstream ifstreamDisabledHELPDiscuss(strFileLoc + "DisabledHELPDiscuss.RDconf");
		if (ifstreamDisabledHELPDiscuss)
		{
			long long Discuss;
			while (ifstreamDisabledHELPDiscuss >> Discuss)
			{
				DisabledHELPDiscuss.insert(Discuss);
			}
		}
		ifstreamDisabledHELPDiscuss.close();
		ifstream ifstreamDisabledOBGroup(strFileLoc + "DisabledOBGroup.RDconf");
		if (ifstreamDisabledOBGroup)
		{
			long long Group;
			while (ifstreamDisabledOBGroup >> Group)
			{
				DisabledOBGroup.insert(Group);
			}
		}
		ifstreamDisabledOBGroup.close();
		ifstream ifstreamDisabledOBDiscuss(strFileLoc + "DisabledOBDiscuss.RDconf");
		if (ifstreamDisabledOBDiscuss)
		{
			long long Discuss;
			while (ifstreamDisabledOBDiscuss >> Discuss)
			{
				DisabledOBDiscuss.insert(Discuss);
			}
		}
		ifstreamDisabledOBDiscuss.close();
		ifstream ifstreamObserveGroup(strFileLoc + "ObserveGroup.RDconf");
		if (ifstreamObserveGroup)
		{
			long long Group, QQ;
			while (ifstreamObserveGroup >> Group >> QQ)
			{
				ObserveGroup.insert(make_pair(Group, QQ));
			}
		}
		ifstreamObserveGroup.close();

		ifstream ifstreamObserveDiscuss(strFileLoc + "ObserveDiscuss.RDconf");
		if (ifstreamObserveDiscuss)
		{
			long long Discuss, QQ;
			while (ifstreamObserveDiscuss >> Discuss >> QQ)
			{
				ObserveDiscuss.insert(make_pair(Discuss, QQ));
			}
		}
		ifstreamObserveDiscuss.close();
		ifstream ifstreamDefault(strFileLoc + "Default.RDconf");
		if (ifstreamDefault)
		{
			long long QQ;
			int DefVal;
			while (ifstreamDefault >> QQ >> DefVal)
			{
				DefaultDice[QQ] = DefVal;
			}
		}
		ifstreamDefault.close();
		ifstream ifstreamWelcomeMsg(strFileLoc + "WelcomeMsg.RDconf");
		if (ifstreamWelcomeMsg)
		{
			long long GroupID;
			string Msg;
			while (ifstreamWelcomeMsg >> GroupID >> Msg)
			{
				while (Msg.find("{space}") != string::npos)Msg.replace(Msg.find("{space}"), 7, " ");
				while (Msg.find("{tab}") != string::npos)Msg.replace(Msg.find("{tab}"), 5, "\t");
				while (Msg.find("{endl}") != string::npos)Msg.replace(Msg.find("{endl}"), 6, "\n");
				while (Msg.find("{enter}") != string::npos)Msg.replace(Msg.find("{enter}"), 7, "\r");
				WelcomeMsg[GroupID] = Msg;
			}
		}
		ifstreamWelcomeMsg.close();
		ilInitList = make_unique<Initlist>(strFileLoc + "INIT.DiceDB");
		ifstream ifstreamCustomMsg(strFileLoc + "CustomMsg.json");
		if (ifstreamCustomMsg)
		{
			ReadCustomMsg(ifstreamCustomMsg);
		}
		ifstreamCustomMsg.close();
	}
	void EventHandler::HandleMsgEvent(DiceMsg dice_msg, bool& block_msg)
	{
		init(dice_msg.msg);
		while (isspace(static_cast<unsigned char>(dice_msg.msg[0])))
			dice_msg.msg.erase(dice_msg.msg.begin());
		string strAt = "[CQ:at,qq=" + to_string(getLoginQQ()) + "]";
		if (dice_msg.msg.substr(0, 6) == "[CQ:at")
		{
			if (dice_msg.msg.substr(0, strAt.length()) != strAt)
			{
				return;
			}
			dice_msg.msg = dice_msg.msg.substr(strAt.length());
		}
		init2(dice_msg.msg);
		if (dice_msg.msg[0] != '.')
			return;
		int intMsgCnt = 1;
		while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
			intMsgCnt++;
		block_msg = true;
		const string strNickName = getName(dice_msg.qq_id, dice_msg.group_id);
		string strLowerMessage = dice_msg.msg;
		transform(strLowerMessage.begin(), strLowerMessage.end(), strLowerMessage.begin(), [](unsigned char c) { return tolower(c); });
		if (strLowerMessage.substr(intMsgCnt, 3) == "bot")
		{
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string Command;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !isspace(
				static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				Command += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string QQNum = strLowerMessage.substr(intMsgCnt, dice_msg.msg.find(' ', intMsgCnt) - intMsgCnt);
			if (Command == "on")
			{
				if (QQNum.empty() || QQNum == to_string(getLoginQQ()) || (QQNum.length() == 4 && QQNum == to_string(
					getLoginQQ() % 10000)))
				{
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
						{
							if (DisabledGroup.count(dice_msg.group_id))
							{
								DisabledGroup.erase(dice_msg.group_id);
								dice_msg.Reply(GlobalMsg["strSuccessfullyEnabledNotice"]);
							}
							else
							{
								dice_msg.Reply(GlobalMsg["strAlreadyEnabledErr"]);
							}
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
						}
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						if (DisabledDiscuss.count(dice_msg.group_id))
						{
							DisabledDiscuss.erase(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strSuccessfullyEnabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strAlreadyEnabledErr"]);
						}
					}
				}
			}
			else if (Command == "off")
			{
				if (QQNum.empty() || QQNum == to_string(getLoginQQ()) || (QQNum.length() == 4 && QQNum == to_string(
					getLoginQQ() % 10000)))
				{
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
						{
							if (!DisabledGroup.count(dice_msg.group_id))
							{
								DisabledGroup.insert(dice_msg.group_id);
								dice_msg.Reply(GlobalMsg["strSuccessfullyDisabledNotice"]);
							}
							else
							{
								dice_msg.Reply(GlobalMsg["strAlreadyDisabledErr"]);
							}
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
						}
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						if (!DisabledDiscuss.count(dice_msg.group_id))
						{
							DisabledDiscuss.insert(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strSuccessfullyDisabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strAlreadyDisabledErr"]);
						}
					}

				}
			}
			else
			{
				if (QQNum.empty() || QQNum == to_string(getLoginQQ()) || (QQNum.length() == 4 && QQNum == to_string(
					getLoginQQ() % 10000)))
				{
					dice_msg.Reply(Dice_Full_Ver);
				}
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 7) == "dismiss")
		{
			intMsgCnt += 7;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string QQNum;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				QQNum += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (QQNum.empty() || (QQNum.length() == 4 && QQNum == to_string(getLoginQQ() % 10000)) || QQNum ==
				to_string(getLoginQQ()))
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						setGroupLeave(dice_msg.group_id);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					setDiscussLeave(dice_msg.group_id);
				}
				else
				{
					return;
				}

			}
		}
		else if ((dice_msg.msg_type==Dice::MsgType::Group && DisabledGroup.count(dice_msg.group_id)) || (dice_msg.msg_type == Dice::MsgType::Discuss && DisabledDiscuss.count(dice_msg.group_id))) return;
		else if (strLowerMessage.substr(intMsgCnt, 4) == "help")
		{
			intMsgCnt += 4;
			while (strLowerMessage[intMsgCnt] == ' ')
				intMsgCnt++;
			const string strAction = strLowerMessage.substr(intMsgCnt);
			if (strAction == "on")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (DisabledHELPGroup.count(dice_msg.group_id))
						{
							DisabledHELPGroup.erase(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strHelpCommandSuccessfullyEnabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strHelpCommandAlreadyEnabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (DisabledHELPDiscuss.count(dice_msg.group_id))
					{
						DisabledHELPDiscuss.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strHelpCommandSuccessfullyEnabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strHelpCommandAlreadyEnabledErr"]);
					}
				}
				return;
			}
			if (strAction == "off")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (!DisabledHELPGroup.count(dice_msg.group_id))
						{
							DisabledHELPGroup.insert(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strHelpCommandSuccessfullyDisabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strHelpCommandAlreadyDisabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (!DisabledHELPDiscuss.count(dice_msg.group_id))
					{
						DisabledHELPDiscuss.insert(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strHelpCommandSuccessfullyDisabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strHelpCommandAlreadyDisabledErr"]);
					}
				}
				return;
			}
			if ( (dice_msg.msg_type == Dice::MsgType::Group && DisabledHELPGroup.count(dice_msg.group_id)) || (dice_msg.msg_type == Dice::MsgType::Group && DisabledHELPDiscuss.count(dice_msg.group_id)) )
			{
				dice_msg.Reply(GlobalMsg["strHelpCommandDisabledErr"]);
				return;
			}
			dice_msg.Reply(GlobalMsg["strHelpMsg"]);
		}
		else if (strLowerMessage.substr(intMsgCnt, 7) == "welcome")
		{
			if (dice_msg.msg_type != Dice::MsgType::Group) return;
			intMsgCnt += 7;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
			{
				string strWelcomeMsg = dice_msg.msg.substr(intMsgCnt);
				if (strWelcomeMsg.empty())
				{
					if (WelcomeMsg.count(dice_msg.group_id))
					{
						WelcomeMsg.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strWelcomeMsgClearedNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strWelcomeMsgIsEmptyErr"]);
					}
				}
				else
				{
					WelcomeMsg[dice_msg.group_id] = strWelcomeMsg;
					dice_msg.Reply(GlobalMsg["strWelcomeMsgUpdatedNotice"]);
				}
			}
			else
			{
				dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "st")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			if (intMsgCnt == strLowerMessage.length())
			{
				dice_msg.Reply(GlobalMsg["strStErr"]);
				return;
			}
			if (strLowerMessage.substr(intMsgCnt, 3) == "clr")
			{
				if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)))
				{
					CharacterProp.erase(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id));
				}
				dice_msg.Reply(GlobalMsg["strPropCleared"]);
				return;
			}
			if (strLowerMessage.substr(intMsgCnt, 3) == "del")
			{
				intMsgCnt += 3;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strSkillName;
				while (intMsgCnt != strLowerMessage.length() && !isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !(strLowerMessage[
					intMsgCnt] == '|'))
				{
					strSkillName += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
					if (SkillNameReplace.count(strSkillName))strSkillName = SkillNameReplace[strSkillName];
					if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) && CharacterProp[SourceType(
						dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count(strSkillName))
					{
						CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].erase(strSkillName);
						dice_msg.Reply(GlobalMsg["strPropDeleted"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPropNotFound"]);
					}
					return;
			}
			if (strLowerMessage.substr(intMsgCnt, 4) == "show")
			{
				intMsgCnt += 4;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strSkillName;
				while (intMsgCnt != strLowerMessage.length() && !isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !(strLowerMessage[
					intMsgCnt] == '|'))
				{
					strSkillName += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (SkillNameReplace.count(strSkillName))
					strSkillName = SkillNameReplace[strSkillName];
				else if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) 
					&& CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count(strSkillName))
				{
					dice_msg.Reply(format(GlobalMsg["strProp"], {strNickName, strSkillName, 
						to_string(CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName])}));
				}
				else if (SkillDefaultVal.count(strSkillName))
				{
					dice_msg.Reply(format(GlobalMsg["strProp"], { strNickName, strSkillName, to_string(SkillDefaultVal[strSkillName]) }));
				}
				else if (!strSkillName.length())/*mark*/
				{
					string strReply = "根据客户档案上的记录，" + strNickName + "的属性如下：";
					map<string, int> AllSkill = CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)];
					if (AllSkill.empty())
					{
						dice_msg.Reply(strNickName + "好像没有录入过和默认值不一样的信息诶");
						return;
					}
					map<string, int>::iterator SkillCount = AllSkill.begin();
					while (!(SkillCount == AllSkill.end()))
					{
						strReply = strReply + " " + SkillCount->first + to_string(SkillCount->second);
						SkillCount++;
					}
					dice_msg.Reply(strReply);
				}
				else
				{
					dice_msg.Reply(GlobalMsg["strPropNotFound"]);
				}
				return;
			}
			bool boolError = false;
			while (intMsgCnt != strLowerMessage.length())
			{
				string strSkillName;
				while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !
					isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && strLowerMessage[intMsgCnt] != '=' && strLowerMessage[intMsgCnt]
					!= ':')
				{
					strSkillName += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (SkillNameReplace.count(strSkillName))strSkillName = SkillNameReplace[strSkillName];
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) || strLowerMessage[intMsgCnt] == '=' || strLowerMessage[intMsgCnt
				] == ':')intMsgCnt++;
				string strSkillVal;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strSkillVal += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (strSkillName.empty() || strSkillVal.empty() || strSkillVal.length() > 3)
				{
					boolError = true;
					break;
				}
				if(!(stoi(strSkillVal) == SkillDefaultVal[strSkillName]))
				CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName] = stoi(strSkillVal);
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) || strLowerMessage[intMsgCnt] == '|')intMsgCnt++;
			}
			if (boolError)
			{
				dice_msg.Reply(GlobalMsg["strPropErr"]);
			}
			else
			{
				dice_msg.Reply(GlobalMsg["strSetPropSuccess"]);
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "ri")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
			}
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))intMsgCnt++;
			string strinit = "D20";
			if (strLowerMessage[intMsgCnt] == '+' || strLowerMessage[intMsgCnt] == '-')
			{
				strinit += strLowerMessage[intMsgCnt];
				intMsgCnt++;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))intMsgCnt++;
			}
			else if (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				strinit += '+';
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strinit += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				intMsgCnt++;
			}
			string strname = dice_msg.msg.substr(intMsgCnt);
			if (strname.empty())
				strname = strNickName;
			else
				strname = strip(strname);
			RD initdice(strinit);
			const int intFirstTimeRes = initdice.Roll();
			if (intFirstTimeRes == Value_Err)
			{
				dice_msg.Reply(GlobalMsg["strValueErr"]);
				return;
			}
			if (intFirstTimeRes == Input_Err)
			{
				dice_msg.Reply(GlobalMsg["strInputErr"]);
				return;
			}
			if (intFirstTimeRes == ZeroDice_Err)
			{
				dice_msg.Reply(GlobalMsg["strZeroDiceErr"]);
				return;
			}
			if (intFirstTimeRes == ZeroType_Err)
			{
				dice_msg.Reply(GlobalMsg["strZeroTypeErr"]);
				return;
			}
			if (intFirstTimeRes == DiceTooBig_Err)
			{
				dice_msg.Reply(GlobalMsg["strDiceTooBigErr"]);
				return;
			}
			if (intFirstTimeRes == TypeTooBig_Err)
			{
				dice_msg.Reply(GlobalMsg["strTypeTooBigErr"]);
				return;
			}
			if (intFirstTimeRes == AddDiceVal_Err)
			{
				dice_msg.Reply(GlobalMsg["strAddDiceValErr"]);
				return;
			}
			if (intFirstTimeRes != 0)
			{
				dice_msg.Reply(GlobalMsg["strUnknownErr"]);
				return;
			}
			ilInitList->insert(dice_msg.group_id, initdice.intTotal, strname);
			const string strReply = strname + "的先攻骰点：" + strinit + '=' + to_string(initdice.intTotal);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 4) == "init")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
			}
			intMsgCnt += 4;
			string strReply;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))intMsgCnt++;
			if (strLowerMessage.substr(intMsgCnt, 3) == "clr")
			{
				if (ilInitList->clear(dice_msg.group_id))
					strReply = GlobalMsg["strInitListClearedNotice"];
				else
					strReply = GlobalMsg["strInitListIsEmptyErr"];
				dice_msg.Reply(strReply);
				return;
			}
			ilInitList->show(dice_msg.group_id, strReply);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage[intMsgCnt] == 'w')
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt++;
			bool boolDetail = false;
			if (strLowerMessage[intMsgCnt] == 'w')
			{
				intMsgCnt++;
				boolDetail = true;
			}
			bool isHidden = false;
			if (strLowerMessage[intMsgCnt] == 'h')
			{
				isHidden = true;
				intMsgCnt += 1;
			}
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;

			int intTmpMsgCnt;
			for (intTmpMsgCnt = intMsgCnt; intTmpMsgCnt != dice_msg.msg.length() && dice_msg.msg[intTmpMsgCnt] != ' ';
				intTmpMsgCnt++)
			{
				if (!isdigit(static_cast<unsigned char>(strLowerMessage[intTmpMsgCnt])) && strLowerMessage[intTmpMsgCnt] != 'd' && strLowerMessage[
					intTmpMsgCnt] != 'k' && strLowerMessage[intTmpMsgCnt] != 'p' && strLowerMessage[intTmpMsgCnt] != 'b'
						&&
						strLowerMessage[intTmpMsgCnt] != 'f' && strLowerMessage[intTmpMsgCnt] != '+' && strLowerMessage[
							intTmpMsgCnt
						] != '-' && strLowerMessage[intTmpMsgCnt] != '#' && strLowerMessage[intTmpMsgCnt] != 'a')
				{
					break;
				}
			}
			string strMainDice = strLowerMessage.substr(intMsgCnt, intTmpMsgCnt - intMsgCnt);
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intTmpMsgCnt])))
				intTmpMsgCnt++;
			string strReason = dice_msg.msg.substr(intTmpMsgCnt);


			int intTurnCnt = 1;
			if (strMainDice.find("#") != string::npos)
			{
				string strTurnCnt = strMainDice.substr(0, strMainDice.find("#"));
				if (strTurnCnt.empty())
					strTurnCnt = "1";
				strMainDice = strMainDice.substr(strMainDice.find("#") + 1);
				RD rdTurnCnt(strTurnCnt, dice_msg.qq_id);
				const int intRdTurnCntRes = rdTurnCnt.Roll();
				if (intRdTurnCntRes == Value_Err)
				{
					dice_msg.Reply(GlobalMsg["strValueErr"]);
					return;
				}
				if (intRdTurnCntRes == Input_Err)
				{
					dice_msg.Reply(GlobalMsg["strInputErr"]);
					return;
				}
				if (intRdTurnCntRes == ZeroDice_Err)
				{
					dice_msg.Reply(GlobalMsg["strZeroDiceErr"]);
					return;
				}
				if (intRdTurnCntRes == ZeroType_Err)
				{
					dice_msg.Reply(GlobalMsg["strZeroTypeErr"]);
					return;
				}
				if (intRdTurnCntRes == DiceTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strDiceTooBigErr"]);
					return;
				}
				if (intRdTurnCntRes == TypeTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strTypeTooBigErr"]);
					return;
				}
				if (intRdTurnCntRes == AddDiceVal_Err)
				{
					dice_msg.Reply(GlobalMsg["strAddDiceValErr"]);
					return;
				}
				if (intRdTurnCntRes != 0)
				{
					dice_msg.Reply(GlobalMsg["strUnknownErr"]);
					return;
				}
				if (rdTurnCnt.intTotal > 10)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeExceeded"]);
					return;
				}
				if (rdTurnCnt.intTotal <= 0)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeErr"]);
					return;
				}
				intTurnCnt = rdTurnCnt.intTotal;
				if (strTurnCnt.find("d") != string::npos)
				{
					string strTurnNotice = strNickName + "的掷骰轮数: " + rdTurnCnt.FormShortString() + "轮";
					if (!isHidden)
					{
						dice_msg.Reply(strTurnNotice);
					}
					else
					{
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							strTurnNotice = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strTurnNotice;
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							strTurnNotice = "在多人聊天中 " + strTurnNotice;
						}
						AddMsgToQueue(Dice::DiceMsg(strTurnNotice, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
						pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							range = ObserveGroup.equal_range(dice_msg.group_id);
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							range = ObserveDiscuss.equal_range(dice_msg.group_id);
						}
						for (auto it = range.first; it != range.second; ++it)
						{
							if (it->second != dice_msg.qq_id)
							{
								AddMsgToQueue(Dice::DiceMsg(strTurnNotice, 0LL, it->second, Dice::MsgType::Private));
							}
						}
					}
				}
			}
			if (strMainDice.empty())
			{
				dice_msg.Reply(GlobalMsg["strEmptyWWDiceErr"]);
				return;
			}
			string strFirstDice = strMainDice.substr(0, strMainDice.find('+') < strMainDice.find('-')
				? strMainDice.find('+')
				: strMainDice.find('-'));
			bool boolAdda10 = true;
			for (auto i : strFirstDice)
			{
				if (!isdigit(static_cast<unsigned char>(i)))
				{
					boolAdda10 = false;
					break;
				}
			}
			if (boolAdda10)
				strMainDice.insert(strFirstDice.length(), "a10");
			RD rdMainDice(strMainDice, dice_msg.qq_id);

			const int intFirstTimeRes = rdMainDice.Roll();
			if (intFirstTimeRes == Value_Err)
			{
				dice_msg.Reply(GlobalMsg["strValueErr"]);
				return;
			}
			if (intFirstTimeRes == Input_Err)
			{
				dice_msg.Reply(GlobalMsg["strInputErr"]);
				return;
			}
			if (intFirstTimeRes == ZeroDice_Err)
			{
				dice_msg.Reply(GlobalMsg["strZeroDiceErr"]);
				return;
			}
			else
			{
				if (intFirstTimeRes == ZeroType_Err)
				{
					dice_msg.Reply(GlobalMsg["strZeroTypeErr"]);
					return;
				}
				if (intFirstTimeRes == DiceTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strDiceTooBigErr"]);
					return;
				}
				if (intFirstTimeRes == TypeTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strTypeTooBigErr"]);
					return;
				}
				if (intFirstTimeRes == AddDiceVal_Err)
				{
					dice_msg.Reply(GlobalMsg["strAddDiceValErr"]);
					return;
				}
				if (intFirstTimeRes != 0)
				{
					dice_msg.Reply(GlobalMsg["strUnknownErr"]);
					return;
				}
			}
			if (!boolDetail && intTurnCnt != 1)
			{
				string strAns = strNickName + "骰出了: " + to_string(intTurnCnt) + "次" + rdMainDice.strDice + ": { ";
				if (!strReason.empty())
					strAns.insert(0, "由于" + strReason + " ");
				vector<int> vintExVal;
				while (intTurnCnt--)
				{
					// 此处返回值无用
					// ReSharper disable once CppExpressionWithoutSideEffects
					rdMainDice.Roll();
					strAns += to_string(rdMainDice.intTotal);
					if (intTurnCnt != 0)
						strAns += ",";
					if ((rdMainDice.strDice == "D100" || rdMainDice.strDice == "1D100") && (rdMainDice.intTotal <= 5 ||
						rdMainDice.intTotal >= 96))
						vintExVal.push_back(rdMainDice.intTotal);
				}
				strAns += " }";
				if (!vintExVal.empty())
				{
					strAns += ",极值: ";
					for (auto it = vintExVal.cbegin(); it != vintExVal.cend(); ++it)
					{
						strAns += to_string(*it);
						if (it != vintExVal.cend() - 1)
							strAns += ",";
					}
				}
				if (!isHidden)
				{
					dice_msg.Reply(strAns);
				}
				else
				{
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						strAns = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strAns;
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						strAns = "在多人聊天中 " + strAns;
					}
					AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
					pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						range = ObserveGroup.equal_range(dice_msg.group_id);
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						range = ObserveDiscuss.equal_range(dice_msg.group_id);
					}
					for (auto it = range.first; it != range.second; ++it)
					{
						if (it->second != dice_msg.qq_id)
						{
							AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, it->second, Dice::MsgType::Private));
						}
					}
				}
			}
			else
			{
				while (intTurnCnt--)
				{
					// 此处返回值无用
					// ReSharper disable once CppExpressionWithoutSideEffects
					rdMainDice.Roll();
					string strAns = strNickName + "骰出了: " + (boolDetail
						? rdMainDice.FormCompleteString()
						: rdMainDice.FormShortString());
					if (!strReason.empty())
						strAns.insert(0, "由于" + strReason + " ");
					if (!isHidden)
					{
						dice_msg.Reply(strAns);
					}
					else
					{
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							strAns = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strAns;
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							strAns = "在多人聊天中 " + strAns;
						}
						AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
						pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							range = ObserveGroup.equal_range(dice_msg.group_id);
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							range = ObserveDiscuss.equal_range(dice_msg.group_id);
						}
						for (auto it = range.first; it != range.second; ++it)
						{
							if (it->second != dice_msg.qq_id)
							{
								AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, it->second, Dice::MsgType::Private));
							}
						}
					}
				}
			}
			if (isHidden)
			{
				const string strReply = strNickName + "进行了一次暗骰";
				dice_msg.Reply(strReply);
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "ob")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
			}
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			const string Command = strLowerMessage.substr(intMsgCnt, dice_msg.msg.find(' ', intMsgCnt) - intMsgCnt);
			if (Command == "on")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (DisabledOBGroup.count(dice_msg.group_id))
						{
							DisabledOBGroup.erase(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strObCommandSuccessfullyEnabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strObCommandAlreadyEnabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (DisabledOBDiscuss.count(dice_msg.group_id))
					{
						DisabledOBDiscuss.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strObCommandSuccessfullyEnabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strObCommandAlreadyEnabledErr"]);
					}
				}

				return;
			}
			if (Command == "off")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (!DisabledOBGroup.count(dice_msg.group_id))
						{
							DisabledOBGroup.insert(dice_msg.group_id);
							ObserveGroup.erase(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strObCommandSuccessfullyDisabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strObCommandAlreadyDisabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (!DisabledOBDiscuss.count(dice_msg.group_id))
					{
						DisabledOBDiscuss.insert(dice_msg.group_id);
						ObserveDiscuss.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strObCommandSuccessfullyDisabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strObCommandAlreadyDisabledErr"]);
					}
				}
				return;
			}
			if ( (dice_msg.msg_type == Dice::MsgType::Group && DisabledOBGroup.count(dice_msg.group_id)) || (dice_msg.msg_type == Dice::MsgType::Discuss && DisabledOBDiscuss.count(dice_msg.group_id)))
			{
				dice_msg.Reply(GlobalMsg["strObCommandDisabledErr"]);
				return;
			}
			if (Command == "list")
			{
				string Msg = "当前的旁观者有:";
				pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					range = ObserveGroup.equal_range(dice_msg.group_id);
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					range = ObserveDiscuss.equal_range(dice_msg.group_id);
				}
				for (auto it = range.first; it != range.second; ++it)
				{
					Msg += "\n" + getName(it->second, dice_msg.group_id) + "(" + to_string(it->second) + ")";
				}
				const string strReply = Msg == "当前的旁观者有:" ? "当前暂无旁观者" : Msg;
				dice_msg.Reply(strReply);
			}
			else if (Command == "clr")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						ObserveGroup.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strObListClearedNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					ObserveDiscuss.erase(dice_msg.group_id);
					dice_msg.Reply(GlobalMsg["strObListClearedNotice"]);
				}

			}
			else if (Command == "exit")
			{
				pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					range = ObserveGroup.equal_range(dice_msg.group_id);
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					range = ObserveDiscuss.equal_range(dice_msg.group_id);
				}
				for (auto it = range.first; it != range.second; ++it)
				{
					if (it->second == dice_msg.qq_id)
					{
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							ObserveGroup.erase(it);
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							ObserveDiscuss.erase(it);
						}
						const string strReply = strNickName + "成功退出旁观模式!";
						dice_msg.Reply(strReply);
						return;
					}
				}
				const string strReply = strNickName + "没有加入旁观模式!";
				dice_msg.Reply(strReply);
			}
			else
			{
				pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					range = ObserveGroup.equal_range(dice_msg.group_id);
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					range = ObserveDiscuss.equal_range(dice_msg.group_id);
				}
				for (auto it = range.first; it != range.second; ++it)
				{
					if (it->second == dice_msg.qq_id)
					{
						const string strReply = strNickName + "已经处于旁观模式!";
						dice_msg.Reply(strReply);
						return;
					}
				}
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					ObserveGroup.insert(make_pair(dice_msg.group_id, dice_msg.qq_id));
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					ObserveDiscuss.insert(make_pair(dice_msg.group_id, dice_msg.qq_id));
				}
				
				const string strReply = strNickName + "成功加入旁观模式!";
				dice_msg.Reply(strReply);
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "ti")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			string strAns = strNickName + "的疯狂发作-临时症状:\n";
			TempInsane(strAns);
			dice_msg.Reply(strAns);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "tz")/*随机特性的部分*/
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			const int intcharacteristic = RandomGenerator::Randint(1, 120);
			string CharacterReply = strNickName + "的特质：\n" + Characteristic[intcharacteristic];
			dice_msg.Reply(CharacterReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "li")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			string strAns = strNickName + "的疯狂发作-总结症状:\n";
			LongInsane(strAns);
			dice_msg.Reply(strAns);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "sc")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string SanCost = strLowerMessage.substr(intMsgCnt, dice_msg.msg.find(' ', intMsgCnt) - intMsgCnt);
			intMsgCnt += SanCost.length();
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string San;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				San += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (SanCost.empty() || SanCost.find("/") == string::npos)
			{
				dice_msg.Reply(GlobalMsg["strSCInvalid"]);
				return;
			}
			if (San.empty() && !(CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) && CharacterProp[
				SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count("理智")))
			{
				dice_msg.Reply(GlobalMsg["strSanInvalid"]);
				return;
			}
				for (const auto& character : SanCost.substr(0, SanCost.find("/")))
				{
					if (!isdigit(static_cast<unsigned char>(character)) && character != 'D' && character != 'd' && character != '+' && character != '-')
					{
						dice_msg.Reply(GlobalMsg["strSCInvalid"]);
						return;
					}
				}
				for (const auto& character : SanCost.substr(SanCost.find("/") + 1))
				{
					if (!isdigit(static_cast<unsigned char>(character)) && character != 'D' && character != 'd' && character != '+' && character != '-')
					{
						dice_msg.Reply(GlobalMsg["strSCInvalid"]);
						return;
					}
				}
				RD rdSuc(SanCost.substr(0, SanCost.find("/")));
				RD rdFail(SanCost.substr(SanCost.find("/") + 1));
				if (rdSuc.Roll() != 0 || rdFail.Roll() != 0)
				{
					dice_msg.Reply(GlobalMsg["strSCInvalid"]);
					return;
				}
				if (San.length() >= 3)
				{
					dice_msg.Reply(GlobalMsg["strSanInvalid"]);
					return;
				}
				const int intSan = San.empty() ? CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)]["理智"] : stoi(San);
				if (intSan == 0)
				{
					dice_msg.Reply(GlobalMsg["strSanInvalid"]);
					return;
				}
				string strAns = strNickName + "的Sancheck:\n1D100=";
				const int intTmpRollRes = RandomGenerator::Randint(1, 100);
				strAns += to_string(intTmpRollRes);

				if (intTmpRollRes <= intSan)
				{
					strAns += " " + GlobalMsg["strSCSuccess"] + "\n你的San值减少" + SanCost.substr(0, SanCost.find("/"));
					if (SanCost.substr(0, SanCost.find("/")).find("d") != string::npos)
						strAns += "=" + to_string(rdSuc.intTotal);
					strAns += +"点,当前剩余" + to_string(max(0, intSan - rdSuc.intTotal)) + "点";
					if (San.empty())
					{
						CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)]["理智"] = max(0, intSan - rdSuc.intTotal);
					}
				}
				else if (intTmpRollRes == 100 || (intSan < 50 && intTmpRollRes > 95))
				{
					strAns += " " + GlobalMsg["strSCFumble"] + "\n你的San值减少" + SanCost.substr(SanCost.find("/") + 1);
					// ReSharper disable once CppExpressionWithoutSideEffects
					rdFail.Max();
					if (SanCost.substr(SanCost.find("/") + 1).find("d") != string::npos)
						strAns += "最大值=" + to_string(rdFail.intTotal);
					strAns += +"点,当前剩余" + to_string(max(0, intSan - rdFail.intTotal)) + "点";
					if (San.empty())
					{
						CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)]["理智"] = max(0, intSan - rdFail.intTotal);
					}
				}
				else
				{
					strAns += " " + GlobalMsg["strSCFailure"] + "\n你的San值减少" + SanCost.substr(SanCost.find("/") + 1);
					if (SanCost.substr(SanCost.find("/") + 1).find("d") != string::npos)
						strAns += "=" + to_string(rdFail.intTotal);
					strAns += +"点,当前剩余" + to_string(max(0, intSan - rdFail.intTotal)) + "点";
					if (San.empty())
					{
						CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)]["理智"] = max(0, intSan - rdFail.intTotal);
					}
				}
				dice_msg.Reply(strAns);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "en")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strSkillName;
			while (intMsgCnt != dice_msg.msg.length() && !isdigit(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])) && !isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt]))
				)
			{
				strSkillName += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (SkillNameReplace.count(strSkillName))strSkillName = SkillNameReplace[strSkillName];
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string strCurrentValue;
			while (isdigit(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
			{
				strCurrentValue += dice_msg.msg[intMsgCnt];
				intMsgCnt++;
			}
			int intCurrentVal;
			if (strCurrentValue.empty())
			{
				if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) && CharacterProp[SourceType(
					dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count(strSkillName))
				{
					intCurrentVal = CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName];
				}
				else if (SkillDefaultVal.count(strSkillName))
				{
					intCurrentVal = SkillDefaultVal[strSkillName];
				}
				else
				{
					dice_msg.Reply(GlobalMsg["strEnValInvalid"]);
					return;
				}
			}
			else
			{
				if (strCurrentValue.length() > 3)
				{
					dice_msg.Reply(GlobalMsg["strEnValInvalid"]);

					return;
				}
				intCurrentVal = stoi(strCurrentValue);
			}

			string strAns = strNickName + "的" + strSkillName + "增强或成长检定:\n1D100=";
			const int intTmpRollRes = RandomGenerator::Randint(1, 100);
			strAns += to_string(intTmpRollRes) + "/" + to_string(intCurrentVal);

			if (intTmpRollRes <= intCurrentVal && intTmpRollRes <= 95)
			{
				strAns += " " + GlobalMsg["strENFailure"] + "\n你的" + (strSkillName.empty() ? "属性或技能值" : strSkillName) + "没有变化!";
			}
			else
			{
				strAns += " " + GlobalMsg["strENSuccess"] + "\n你的" + (strSkillName.empty() ? "属性或技能值" : strSkillName) + "增加1D10=";
				const int intTmpRollD10 = RandomGenerator::Randint(1, 10);
				strAns += to_string(intTmpRollD10) + "点,当前为" + to_string(intCurrentVal + intTmpRollD10) + "点";
				if (strCurrentValue.empty())
				{
					CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName] = intCurrentVal +
						intTmpRollD10;
				}
			}
			dice_msg.Reply(strAns);
		}
		else if (strLowerMessage.substr(intMsgCnt, 4) == "jrrp")
		{
			intMsgCnt += 4;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			const string Command = strLowerMessage.substr(intMsgCnt, dice_msg.msg.find(' ', intMsgCnt) - intMsgCnt);
			if (Command == "on")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (DisabledJRRPGroup.count(dice_msg.group_id))
						{
							DisabledJRRPGroup.erase(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strJrrpCommandSuccessfullyEnabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strJrrpCommandAlreadyEnabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (DisabledJRRPDiscuss.count(dice_msg.group_id))
					{
						DisabledJRRPDiscuss.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strJrrpCommandSuccessfullyEnabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strJrrpCommandAlreadyEnabledErr"]);
					}
				}
				return;
			}
			if (Command == "off")
			{
				if (dice_msg.msg_type == Dice::MsgType::Group)
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (!DisabledJRRPGroup.count(dice_msg.group_id))
						{
							DisabledJRRPGroup.insert(dice_msg.group_id);
							dice_msg.Reply(GlobalMsg["strJrrpCommandSuccessfullyDisabledNotice"]);
						}
						else
						{
							dice_msg.Reply(GlobalMsg["strJrrpCommandAlreadyDisabledErr"]);
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
				}
				else if (dice_msg.msg_type == Dice::MsgType::Discuss)
				{
					if (!DisabledJRRPDiscuss.count(dice_msg.group_id))
					{
						DisabledJRRPDiscuss.insert(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strJrrpCommandSuccessfullyDisabledNotice"]);
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strJrrpCommandAlreadyDisabledErr"]);
					}
				}
				return;
			}
			if ((dice_msg.msg_type==Dice::MsgType::Group && DisabledJRRPGroup.count(dice_msg.group_id)) || (dice_msg.msg_type == Dice::MsgType::Discuss && DisabledJRRPDiscuss.count(dice_msg.group_id)) )
			{
				dice_msg.Reply(GlobalMsg["strJrrpCommandDisabledErr"]);
				return;
			}
			string des;
			string data = "QQ=" + to_string(CQ::getLoginQQ()) + "&v=20190114" + "&QueryQQ=" + to_string(dice_msg.qq_id);
			char* frmdata = new char[data.length() + 1];
			strcpy_s(frmdata, data.length() + 1, data.c_str());
			bool res = Network::POST("api.kokona.tech", "/jrrp", 5555, frmdata, des);
			delete[] frmdata;
			if (res)
			{
				dice_msg.Reply(format(GlobalMsg["strJrrp"], { strNickName, des }));
			}
			else
			{
				dice_msg.Reply(format(GlobalMsg["strJrrpErr"], { des }));
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 4) == "name")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 4;
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;

			string type;
			while (isalpha(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				type += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}

			auto nameType = NameGenerator::Type::UNKNOWN;
			if (type == "cn")
				nameType = NameGenerator::Type::CN;
			else if (type == "en")
				nameType = NameGenerator::Type::EN;
			else if (type == "jp")
				nameType = NameGenerator::Type::JP;

			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;

			string strNum;
			while (isdigit(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
			{
				strNum += dice_msg.msg[intMsgCnt];
				intMsgCnt++;
			}
			if (strNum.size() > 2)
			{
				dice_msg.Reply(GlobalMsg["strNameNumTooBig"]);
				return;
			}
			int intNum = stoi(strNum.empty() ? "1" : strNum);
			if (intNum > 10)
			{
				dice_msg.Reply(GlobalMsg["strNameNumTooBig"]);
				return;
			}
			if (intNum == 0)
			{
				dice_msg.Reply(GlobalMsg["strNameNumCannotBeZero"]);
				return;
			}
			vector<string> TempNameStorage;
			while (TempNameStorage.size() != intNum)
			{
				string name = NameGenerator::getRandomName(nameType);
				if (find(TempNameStorage.begin(), TempNameStorage.end(), name) == TempNameStorage.end())
				{
					TempNameStorage.push_back(name);
				}
			}
			string strReply = strNickName + "的随机名称:\n";
			for (auto i = 0; i != TempNameStorage.size(); i++)
			{
				strReply.append(TempNameStorage[i]);
				if (i != TempNameStorage.size() - 1)strReply.append(", ");
			}
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "nnn")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string type = strLowerMessage.substr(intMsgCnt, 2);
			string name;
			if (type == "cn")
				name = NameGenerator::getChineseName();
			else if (type == "en")
				name = NameGenerator::getEnglishName();
			else if (type == "jp")
				name = NameGenerator::getJapaneseName();
			else
				name = NameGenerator::getRandomName();
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				Name->set(0LL, dice_msg.qq_id, name);
			}
			else
			{
				Name->set(dice_msg.group_id, dice_msg.qq_id, name);
			}
			const string strReply = "已将" + strNickName + "的" + (dice_msg.msg_type == Dice::MsgType::Private ? "全局" : "") +"名称更改为" + name;
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "nn")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
			}
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string name = dice_msg.msg.substr(intMsgCnt);
			if (name.length() > 50)
			{
				dice_msg.Reply(GlobalMsg["strNameTooLongErr"]);
				return;
			}
			if (!name.empty())
			{
				Name->set(dice_msg.group_id, dice_msg.qq_id, name);
				const string strReply = "已将" + strNickName + "的名称更改为" + strip(name);
				dice_msg.Reply(strReply);
			}
			else
			{
				if (Name->del(dice_msg.group_id, dice_msg.qq_id))
				{
					const string strReply = "已将" + strNickName + "的名称删除";
					dice_msg.Reply(strReply);
				}
				else
				{
					const string strReply = strNickName + GlobalMsg["strNameDelErr"];
					dice_msg.Reply(strReply);
				}
			}
		}
		else if (strLowerMessage[intMsgCnt] == 'n')
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 1;
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string name = dice_msg.msg.substr(intMsgCnt);
			if (name.length() > 50)
			{
				dice_msg.Reply(GlobalMsg["strNameTooLongErr"]);
				return;
			}
			if (!name.empty())
			{
				Name->set(0LL, dice_msg.qq_id, name);
				const string strReply = "已将" + strNickName + "的全局名称更改为" + strip(name);
				dice_msg.Reply(strReply);
			}
			else
			{
				if (Name->del(0LL, dice_msg.qq_id))
				{
					const string strReply = "已将" + strNickName + "的全局名称删除";
					dice_msg.Reply(strReply);
				}
				else
				{
					const string strReply = strNickName + GlobalMsg["strNameDelErr"];
					dice_msg.Reply(strReply);
				}
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 5) == "rules")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 5;
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string strSearch = dice_msg.msg.substr(intMsgCnt);
			for (auto& n : strSearch)
				n = toupper(static_cast<unsigned char>(n));
			string strReturn;
			if (GetRule::analyze(strSearch, strReturn))
			{
				dice_msg.Reply(strReturn);
			}
			else
			{
				dice_msg.Reply(GlobalMsg["strRuleErr"] + strReturn);
			}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "mo")/*备忘录*/
		{
			if (!MsgSend)
			{
				dice_msg.Reply("主循环尚未启动，请等待10秒");
				return;
			}
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string Command;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !isspace(
				static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && (Command.length() < 3))/*获取指令后的3个字符作为子指令*/
			{
				Command += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))/*删除句首的\n*/
				intMsgCnt++;
			string strAction;
			if (Command == "add")
			{
				strAction = strip(dice_msg.msg.substr(intMsgCnt));
				if (strAction.length() == 0)
				{
					dice_msg.Reply("你到底想让我记录什么呀？");
					return;
				}
				while (!(strAction.find("\r") == string::npos))/*\r转化为\n*/
					strAction.replace(strAction.find("\r"), 1, "\n");
				while (!(strAction.find("\n)") == string::npos))/*删除\n之后的)*/
					strAction.erase(strAction.find("\n)") + 1, 1);
				while (!(strAction.find("\n\n") == string::npos))/*删除连续的\n，只保留一个*/
					strAction.erase(strAction.find("\n\n"), 1);
				while (strAction[strAction.length() - 1] == '\n')/*删除句末的\n*/
					strAction.erase(strAction.length() - 1, 1);
				while (!(strAction.find("{memostart}") == string::npos))/*删除标志符*/
					strAction.erase(strAction.find("{memostart}"), 11);
				if (strAction.length() == 0)
				{
					dice_msg.Reply("你到底想让我记录什么呀？");
					return;
				}
				if (strAction.length() > 200)
				{
					dice_msg.Reply("备忘录一条最多只能记录100个中文字符哦");
					return;
				}
				dice_msg.Reply(MemoRecorder(dice_msg.qq_id, strAction, MemoRecoEnum::Add, 0));
				MemoPack(true, false);
			}
			else if (Command == "new")
			{
				strAction = strip(dice_msg.msg.substr(intMsgCnt));
				if (strAction.length() == 0)
				{
					dice_msg.Reply("你到底想让我记录什么呀？");
					return;
				}
				while (!(strAction.find("\r") == string::npos))
					strAction.replace(strAction.find("\r"), 1, "\n");
				while (!(strAction.find("\n)") == string::npos))
					strAction.erase(strAction.find("\n)") + 1, 1);
				while (!(strAction.find("\n\n") == string::npos))
					strAction.erase(strAction.find("\n\n"), 1);
				while (strAction[strAction.length() - 1] == '\n')
					strAction.erase(strAction.length() - 1, 1);
				while (!(strAction.find("{memostart}") == string::npos))/*删除标志符*/
					strAction.erase(strAction.find("{memostart}"), 11);
				if (strAction.length() == 0)
				{
					dice_msg.Reply("你到底想让我记录什么呀？");
					return;
				}
				if (strAction.length() > 200)
				{
					dice_msg.Reply("备忘录一条最多只能记录100个中文字符哦");
					return;
				}
				dice_msg.Reply(MemoRecorder(dice_msg.qq_id, strAction, MemoRecoEnum::New, 0));
				MemoPack(true, false);
			}
			else if (Command == "clr")
			{
				string strMemoNum;
				while (strLowerMessage[intMsgCnt] == '0')
					strLowerMessage.erase(intMsgCnt, 1);
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strMemoNum += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				int intMemoNum;
				if (!strMemoNum.length())
				{
					intMemoNum = 0;
				}
				else
				{
					intMemoNum = stoi(strMemoNum);
				}
				dice_msg.Reply(MemoRecorder(dice_msg.qq_id, strAction, MemoRecoEnum::Clear,intMemoNum));
				MemoPack(true, false);
			}
			else if (Command == "on")
			{
				string strReply = MemoAlarmControl(dice_msg.qq_id, true);
				dice_msg.Reply(strReply);
			}
			else if (Command == "off")
			{
				string strReply = MemoAlarmControl(dice_msg.qq_id, false);
				dice_msg.Reply(strReply);
			}
			else if (Command == "set")
			{
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string AlarmTime;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					AlarmTime += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if ((atoi(AlarmTime.c_str()) >= 24) || AlarmTime == "")
				{
					string strReply = "你这么说我不知道该什么时候提醒你呀";
					dice_msg.Reply(strReply);
					return;
				}
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				if (static_cast<unsigned char>(strLowerMessage[intMsgCnt]) == 'p')
				{
					string strReply = SetMemoAlarm(dice_msg.qq_id, dice_msg.group_id, atoi(AlarmTime.c_str()), true, true);
					dice_msg.Reply(strReply);
				}
				else
				{
					string strReply = SetMemoAlarm(dice_msg.qq_id, dice_msg.group_id, atoi(AlarmTime.c_str()), false, true);
					dice_msg.Reply(strReply);
				}
				MemoPack(true, false);
			}
			else if (Command == "del")
			{
				string strReply = SetMemoAlarm(dice_msg.qq_id, dice_msg.group_id, 0, true, false);
				dice_msg.Reply(strReply);
				MemoPack(true, false);
			}
			else
			{
				string strReply = strNickName;
				strReply += MemoRecorderGet(dice_msg.qq_id);
				dice_msg.Reply(strReply);
			}
			return;
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "me")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
				intMsgCnt += 2;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strGroupID;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strGroupID += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strAction = strip(dice_msg.msg.substr(intMsgCnt));

				for (auto i : strGroupID)
				{
					if (!isdigit(static_cast<unsigned char>(i)))
					{
						dice_msg.Reply(GlobalMsg["strGroupIDInvalid"]);
						return;
					}
				}
				if (strGroupID.empty())
				{
					dice_msg.Reply("你呼叫的群号是空号！请查证后再拨！");
					return;
				}
				if (strAction.empty())
				{
					dice_msg.Reply("你到底想干什么啊？");
					return;
				}
				const long long llGroupID = stoll(strGroupID);
				if (DisabledGroup.count(llGroupID))
				{
					dice_msg.Reply(GlobalMsg["strDisabledErr"]);
					return;
				}
				if (DisabledMEGroup.count(llGroupID))
				{
					dice_msg.Reply(GlobalMsg["strMEDisabledErr"]);
					return;
				}
				string strReply = getName(dice_msg.qq_id, llGroupID) + strAction;
				const int intSendRes = sendGroupMsg(llGroupID, strReply);
				if (intSendRes < 0)
				{
					dice_msg.Reply(GlobalMsg["strSendErr"]);
				}
				else
				{
					dice_msg.Reply("信息已送达！");
				}
				}
			else if ((dice_msg.msg_type == Dice::MsgType::Group)||(dice_msg.msg_type == Dice::MsgType::Discuss))
			{
				intMsgCnt += 2;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strAction = strLowerMessage.substr(intMsgCnt);
				if (strAction == "on")
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (DisabledMEGroup.count(dice_msg.group_id))
						{
							DisabledMEGroup.erase(dice_msg.group_id);
							dice_msg.Reply("成功在本群中启用.me命令!让我们来玩过家家吧");
						}
						else
						{
							dice_msg.Reply("诶？这不是玩的正开心么？（在本群中.me命令没有被禁用！）");
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
					return;
				}
				if (strAction == "off")
				{
					if (getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).permissions >= 2)
					{
						if (!DisabledMEGroup.count(dice_msg.group_id))
						{
							DisabledMEGroup.insert(dice_msg.group_id);
							dice_msg.Reply("成功在本群中禁用.me命令！过家家禁止x");
						}
						else
						{
							dice_msg.Reply("人家才没有在玩过家家！（.me命令没有被启用！）");
						}
					}
					else
					{
						dice_msg.Reply(GlobalMsg["strPermissionDeniedErr"]);
					}
					return;
				}
				if (DisabledMEGroup.count(dice_msg.group_id))
				{
					dice_msg.Reply("在本群中.me命令已被禁用！过家家禁止x");
					return;
				}
				if (DisabledMEGroup.count(dice_msg.group_id))
				{
					dice_msg.Reply(GlobalMsg["strMEDisabledErr"]);
					return;
				}
				strAction = strip(dice_msg.msg.substr(intMsgCnt));
				if (strAction.empty())
				{
					dice_msg.Reply("你到底想干什么呀（动作不能为空!）");
					return;
				}
				const string strReply = strNickName + strAction;
				dice_msg.Reply(strReply);
			}
			else
				{
				return;
				}
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "set")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strDefaultDice = strLowerMessage.substr(intMsgCnt, strLowerMessage.find(" ", intMsgCnt) - intMsgCnt);
			while (strDefaultDice[0] == '0')
				strDefaultDice.erase(strDefaultDice.begin());
			if (strDefaultDice.empty())
				strDefaultDice = "100";
			for (auto charNumElement : strDefaultDice)
				if (!isdigit(static_cast<unsigned char>(charNumElement)))
				{
					dice_msg.Reply(GlobalMsg["strSetInvalid"]);
					return;
				}
			if (strDefaultDice.length() > 5)
			{
				dice_msg.Reply(GlobalMsg["strSetTooBig"]);
				return;
			}
			const int intDefaultDice = stoi(strDefaultDice);
			if (intDefaultDice == 100)
				DefaultDice.erase(dice_msg.qq_id);
			else
				DefaultDice[dice_msg.qq_id] = intDefaultDice;
			const string strSetSuccessReply = "已将" + strNickName + "的默认骰类型更改为D" + strDefaultDice;
			dice_msg.Reply(strSetSuccessReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 5) == "coc6d")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			string strReply = strNickName;
			COC6D(strReply);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 4) == "coc6")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 4;
			if (strLowerMessage[intMsgCnt] == 's')
				intMsgCnt++;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strNum;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strNum += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (strNum.length() > 2)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			const int intNum = stoi(strNum.empty() ? "1" : strNum);
			if (intNum > 10)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			if (intNum == 0)
			{
				dice_msg.Reply(GlobalMsg["strCharacterCannotBeZero"]);
				return;
			}
			string strReply = strNickName;
			COC6(strReply, intNum);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "dnd")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strNum;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strNum += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (strNum.length() > 2)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			const int intNum = stoi(strNum.empty() ? "1" : strNum);
			if (intNum > 10)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			if (intNum == 0)
			{
				dice_msg.Reply(GlobalMsg["strCharacterCannotBeZero"]);
				return;
			}
			string strReply = strNickName;
			DND(strReply, intNum);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 5) == "coc7d" || strLowerMessage.substr(intMsgCnt, 4) == "cocd")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			string strReply = strNickName;
			COC7D(strReply);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "coc")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 3;
			if (strLowerMessage[intMsgCnt] == '7')
				intMsgCnt++;
			if (strLowerMessage[intMsgCnt] == 's')
				intMsgCnt++;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strNum;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strNum += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (strNum.length() > 2)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			const int intNum = stoi(strNum.empty() ? "1" : strNum);
			if (intNum > 10)
			{
				dice_msg.Reply(GlobalMsg["strCharacterTooBig"]);
				return;
			}
			if (intNum == 0)
			{
				dice_msg.Reply(GlobalMsg["strCharacterCannotBeZero"]);
				return;
			}
			string strReply = strNickName;
			COC7(strReply, intNum);
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "ast")/*mark*/
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			if (dice_msg.msg_type == Dice::MsgType::Private) 
				return;
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string Command;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !isspace(
				static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				Command += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (Command == "show")
			{
				string strReply;
				if (!RoomRule.count(dice_msg.group_id))
				{
					strReply = "当前房规为：大成功：1-5 大失败：96-100";
				}
				else if (RoomRule[dice_msg.group_id] == 1)
				{
					strReply = "当前房规为：大成功：1 大失败：100";
				}
				else if (RoomRule[dice_msg.group_id] == 0)
				{
					strReply = "当前房规为：大成功：已禁用 大失败：已禁用";
				}
				else
				{
					strReply = "当前房规为：大成功：1-" + to_string(RoomRule[dice_msg.group_id]) + "大失败：" + to_string(101 - RoomRule[dice_msg.group_id]) + "-100";
				}
				dice_msg.Reply(strReply);
				return;
			}
			else
			{
				string strRoomRule;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strRoomRule += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (!strRoomRule.length())
				{
					if (RoomRule.count(dice_msg.group_id))
					{
						RoomRule.erase(dice_msg.group_id);
						dice_msg.Reply(GlobalMsg["strRoomRuleClear"]);
					}
					else
						dice_msg.Reply(GlobalMsg["strRoomRuleClearErr"]);
					return;
				}
				int intRoomRule = stoi(strRoomRule);
				if (intRoomRule > 50 || intRoomRule < 0)
				{
					dice_msg.Reply(GlobalMsg["strRoomRuleSetErr"]);
				}
				else
				{
					RoomRule[dice_msg.group_id] = intRoomRule;
					string strReply;
					if (intRoomRule == 1)
						strReply = GlobalMsg["strRoomRuleSet"] + "当前房规为：大成功：1  大失败：100";
					else if (!intRoomRule)
						strReply = GlobalMsg["strRoomRuleSet"] + "当前房规为：大成功：已禁用  大失败：已禁用";
					else
						strReply = GlobalMsg["strRoomRuleSet"] + "当前房规为：大成功：1-" + to_string(intRoomRule) + "大失败：" + to_string(101 - intRoomRule) + "-100";
					dice_msg.Reply(strReply);
				}
				return;
							}
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "ra")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 2;
			bool setporb = 0, isPunish = 1;/*下面是修改的部分mark(包括本行*/
			string strpbNum;
			int intpbNum = 1;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			if (strLowerMessage[intMsgCnt] == 'p'|| strLowerMessage[intMsgCnt] == 'b')
			{
				setporb = 1;
				if (strLowerMessage[intMsgCnt] == 'b')
					isPunish = 0;
				intMsgCnt++;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strpbNum += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (strpbNum.length())
					intpbNum = stoi(strpbNum);
				if (intpbNum > 10)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeExceeded"]);
					return;
				}
				if (!intpbNum)
					setporb = 0;
			}/*上面是修改的部分mark(包括本行*/
			string strSkillName;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))intMsgCnt++;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !
				isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && strLowerMessage[intMsgCnt] != '=' && strLowerMessage[intMsgCnt] !=
				':')
			{
				strSkillName += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (SkillNameReplace.count(strSkillName))strSkillName = SkillNameReplace[strSkillName];
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) || strLowerMessage[intMsgCnt] == '=' || strLowerMessage[intMsgCnt] ==
				':')intMsgCnt++;
			string strSkillVal;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strSkillVal += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				intMsgCnt++;
			}
			string strReason = dice_msg.msg.substr(intMsgCnt);
			int intSkillVal;
			if (strSkillVal.empty())
			{
				if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) && CharacterProp[SourceType(
					dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count(strSkillName))
				{
					intSkillVal = CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName];
				}
				else if (SkillDefaultVal.count(strSkillName))
				{
					intSkillVal = SkillDefaultVal[strSkillName];
				}
				else
				{
					dice_msg.Reply(GlobalMsg["strUnknownPropErr"]);
					return;
				}
			}
			else if (strSkillVal.length() > 3)
			{
				dice_msg.Reply(GlobalMsg["strPropErr"]);
				return;
			}
			else
			{
				intSkillVal = stoi(strSkillVal);
			}
			int intD100Res = RandomGenerator::Randint(1, 100);
			string strReply = strNickName + "进行" + strSkillName + "检定: D100=" + to_string(intD100Res);/*下面是修改的部分mark(包括本行*/
			if (setporb)
			{
				int pbRandom, single_figures;
				string pbShow = "";
				if (intD100Res == 100)
					single_figures = 0;
				else
					single_figures = intD100Res % 10;
				if (isPunish)
				{
					pbShow = "（惩罚骰：";
					for (int pbCunt = 0; pbCunt < intpbNum; pbCunt++)
					{
						pbRandom = RandomGenerator::Randint(0, 9);
						pbShow = pbShow + " " + to_string(pbRandom);
						if ((pbRandom == 0) && (single_figures == 0))
							pbRandom = 10;
						pbRandom = pbRandom * 10;
						if (pbRandom > intD100Res)
							intD100Res = pbRandom + single_figures;
					}
				}
				else
				{
					pbShow = "（奖励骰：";
					for (int pbCunt = 0; pbCunt < intpbNum; pbCunt++)
					{
						pbRandom = RandomGenerator::Randint(0, 9);
						pbShow = pbShow + " " + to_string(pbRandom);
						if ((pbRandom == 0) && (single_figures == 0))
							pbRandom = 10;
						pbRandom = pbRandom * 10;
						if (pbRandom < intD100Res)
							intD100Res = pbRandom + single_figures;
					}
				}
				pbShow = pbShow + "），最终结果是：" + to_string(intD100Res);
				strReply += pbShow + "/" + to_string(intSkillVal) + " ";
			}
			else
				strReply += "/" + to_string(intSkillVal) + " ";
			int RoomRuleNum = 5;/*自定义房规部分（包括此行向下*/
			if (RoomRule.count(dice_msg.group_id))
				RoomRuleNum = RoomRule[dice_msg.group_id];
			/*上面是修改的部分mark(包括本行*/

			if (intD100Res <= intSkillVal)/*成功*/
			{
				if (intD100Res <= RoomRuleNum)
				{
					strReply += GlobalMsg["strCriticalSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::CriticalSuccess);
				}
				else if (intD100Res <= intSkillVal / 5)
				{
					strReply += GlobalMsg["strExtremeSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::ExttremeSuccess);
				}
				else if (intD100Res <= intSkillVal / 2)
				{
					strReply += GlobalMsg["strHardSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::HardSuccess);
				}
				else
				{
					strReply += GlobalMsg["strSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Success);
				}
			}
			else/*失败*/
			{
				if (intD100Res >= (101 - RoomRuleNum))
				{
					strReply += GlobalMsg["strFumble"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Fumble);
				}
				else
				{
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Failure);
					strReply += GlobalMsg["strFailure"];
				}
			}

			if (!strReason.empty())
			{
				strReply = "由于" + strReason + " " + strReply;
			}
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "rc")
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 2;
				bool setporb = 0, isPunish = 1;/*下面是修改的部分mark(包括本行*/
			string strpbNum;
			int intpbNum = 1;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			if (strLowerMessage[intMsgCnt] == 'p' || strLowerMessage[intMsgCnt] == 'b')
			{
				setporb = 1;
				if (strLowerMessage[intMsgCnt] == 'b')
					isPunish = 0;
				intMsgCnt++;
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strpbNum += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (strpbNum.length())
					intpbNum = stoi(strpbNum);
				if (intpbNum > 10)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeExceeded"]);
					return;
				}
				if (!intpbNum)
					setporb = 0;
			}/*上面是修改的部分mark(包括本行*/
			string strSkillName;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))intMsgCnt++;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !
				isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && strLowerMessage[intMsgCnt] != '=' && strLowerMessage[intMsgCnt] !=
				':')
			{
				strSkillName += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (SkillNameReplace.count(strSkillName))strSkillName = SkillNameReplace[strSkillName];
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) || strLowerMessage[intMsgCnt] == '=' || strLowerMessage[intMsgCnt] ==
				':')intMsgCnt++;
			string strSkillVal;
			while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				strSkillVal += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				intMsgCnt++;
			}
			string strReason = dice_msg.msg.substr(intMsgCnt);
			int intSkillVal;
			if (strSkillVal.empty())
			{
				if (CharacterProp.count(SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)) && CharacterProp[SourceType(
					dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)].count(strSkillName))
				{
					intSkillVal = CharacterProp[SourceType(dice_msg.qq_id, dice_msg.msg_type, dice_msg.group_id)][strSkillName];
				}
				else if (SkillDefaultVal.count(strSkillName))
				{
					intSkillVal = SkillDefaultVal[strSkillName];
				}
				else
				{
					dice_msg.Reply(GlobalMsg["strUnknownPropErr"]);
					return;
				}
			}
			else if (strSkillVal.length() > 3)
			{
				dice_msg.Reply(GlobalMsg["strPropErr"]);
				return;
			}
			else
			{
				intSkillVal = stoi(strSkillVal);
			}
			int intD100Res = RandomGenerator::Randint(1, 100);
			string strReply = strNickName + "进行" + strSkillName + "检定: D100=" + to_string(intD100Res);/*下面是修改的部分mark(包括本行*/
			if (setporb)
			{
				int pbRandom, single_figures;
				string pbShow = "";
				if (intD100Res == 100)
					single_figures = 0;
				else
					single_figures = intD100Res % 10;
				if (isPunish)
				{
					pbShow = "（惩罚骰：";
					for (int pbCunt = 0; pbCunt < intpbNum; pbCunt++)
					{
						pbRandom = RandomGenerator::Randint(0, 9);
						pbShow = pbShow + " " + to_string(pbRandom);
						if ((pbRandom == 0) && (single_figures == 0))
							pbRandom = 10;
						pbRandom = pbRandom * 10;
						if (pbRandom > intD100Res)
							intD100Res = pbRandom + single_figures;
					}
				}
				else
				{
					pbShow = "（奖励骰：";
					for (int pbCunt = 0; pbCunt < intpbNum; pbCunt++)
					{
						pbRandom = RandomGenerator::Randint(0, 9);
						pbShow = pbShow + " " + to_string(pbRandom);
						if ((pbRandom == 0) && (single_figures == 0))
							pbRandom = 10;
						pbRandom = pbRandom * 10;
						if (pbRandom < intD100Res)
							intD100Res = pbRandom + single_figures;
					}
				}
				pbShow = pbShow + "），最终结果是：" + to_string(intD100Res);
				strReply += pbShow + "/" + to_string(intSkillVal) + " ";
			}
			else
				strReply += "/" + to_string(intSkillVal) + " ";/*上面是修改的部分mark(包括本行*/

			if (intD100Res <= intSkillVal)/*成功*/
			{
				if (intD100Res == 1)
				{
					strReply += GlobalMsg["strCriticalSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::CriticalSuccess);
				}
				else if (intD100Res <= intSkillVal / 5)
				{
					strReply += GlobalMsg["strExtremeSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::ExttremeSuccess);
				}
				else if (intD100Res <= intSkillVal / 2)
				{
					strReply += GlobalMsg["strHardSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::HardSuccess);
				}
				else
				{
					strReply += GlobalMsg["strSuccess"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Success);
				}
			}
			else/*失败*/
			{
				if ((intSkillVal >= 50) && (intD100Res == 100))
				{
					strReply += GlobalMsg["strFumble"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Fumble);
				}
				else if ((intSkillVal < 50) && (intD100Res > 95))
				{
					strReply += GlobalMsg["strFumble"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Fumble);

				}
				else
				{
					strReply += GlobalMsg["strFailure"];
					CheckSumRecorder(dice_msg.qq_id, CheckSumEnum::Failure);
				}
			}
			
			if (!strReason.empty())
			{
				strReply = "由于" + strReason + " " + strReply;
			}
			dice_msg.Reply(strReply);
		}
		else if (strLowerMessage[intMsgCnt] == 'r')
		{
			if (dice_msg.msg_type == Dice::MsgType::Group)
				LastMsgTimeRecorder(dice_msg.group_id);/*最后发言时间记录模块*/
			intMsgCnt += 1;
			bool boolDetail = true, isHidden = false;
			if (dice_msg.msg[intMsgCnt] == 's')
			{
				boolDetail = false;
				intMsgCnt++;
			}
			if (strLowerMessage[intMsgCnt] == 'h')
			{
				isHidden = true;
				intMsgCnt++;
			}
			while (isspace(static_cast<unsigned char>(dice_msg.msg[intMsgCnt])))
				intMsgCnt++;
			string strMainDice;
			string strReason;
			bool tmpContainD = false;
			int intTmpMsgCnt;
			for (intTmpMsgCnt = intMsgCnt; intTmpMsgCnt != dice_msg.msg.length() && dice_msg.msg[intTmpMsgCnt] != ' ';
				intTmpMsgCnt++)
			{
				if (strLowerMessage[intTmpMsgCnt] == 'd' || strLowerMessage[intTmpMsgCnt] == 'p' || strLowerMessage[
					intTmpMsgCnt] == 'b' || strLowerMessage[intTmpMsgCnt] == '#' || strLowerMessage[intTmpMsgCnt] == 'f'
						|| strLowerMessage[intTmpMsgCnt] == 'a')
					tmpContainD = true;
					if (!isdigit(static_cast<unsigned char>(strLowerMessage[intTmpMsgCnt])) && strLowerMessage[intTmpMsgCnt] != 'd' && strLowerMessage[
						intTmpMsgCnt] != 'k' && strLowerMessage[intTmpMsgCnt] != 'p' && strLowerMessage[intTmpMsgCnt] != 'b'
							&&
							strLowerMessage[intTmpMsgCnt] != 'f' && strLowerMessage[intTmpMsgCnt] != '+' && strLowerMessage[
								intTmpMsgCnt
							] != '-' && strLowerMessage[intTmpMsgCnt] != '#' && strLowerMessage[intTmpMsgCnt] != 'a')
					{
						break;
					}
			}
			if (tmpContainD)
			{
				strMainDice = strLowerMessage.substr(intMsgCnt, intTmpMsgCnt - intMsgCnt);
				while (isspace(static_cast<unsigned char>(dice_msg.msg[intTmpMsgCnt])))
					intTmpMsgCnt++;
				strReason = dice_msg.msg.substr(intTmpMsgCnt);
			}
			else
				strReason = dice_msg.msg.substr(intMsgCnt);

			int intTurnCnt = 1;
			if (strMainDice.find("#") != string::npos)
			{
				string strTurnCnt = strMainDice.substr(0, strMainDice.find("#"));
				if (strTurnCnt.empty())
					strTurnCnt = "1";
				strMainDice = strMainDice.substr(strMainDice.find("#") + 1);
				RD rdTurnCnt(strTurnCnt, dice_msg.qq_id);
				const int intRdTurnCntRes = rdTurnCnt.Roll();
				if (intRdTurnCntRes == Value_Err)
				{
					dice_msg.Reply(GlobalMsg["strValueErr"]);
					return;
				}
				if (intRdTurnCntRes == Input_Err)
				{
					dice_msg.Reply(GlobalMsg["strInputErr"]);
					return;
				}
				if (intRdTurnCntRes == ZeroDice_Err)
				{
					dice_msg.Reply(GlobalMsg["strZeroDiceErr"]);
					return;
				}
				if (intRdTurnCntRes == ZeroType_Err)
				{
					dice_msg.Reply(GlobalMsg["strZeroTypeErr"]);
					return;
				}
				if (intRdTurnCntRes == DiceTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strDiceTooBigErr"]);
					return;
				}
				if (intRdTurnCntRes == TypeTooBig_Err)
				{
					dice_msg.Reply(GlobalMsg["strTypeTooBigErr"]);
					return;
				}
				if (intRdTurnCntRes == AddDiceVal_Err)
				{
					dice_msg.Reply(GlobalMsg["strAddDiceValErr"]);
					return;
				}
				if (intRdTurnCntRes != 0)
				{
					dice_msg.Reply(GlobalMsg["strUnknownErr"]);
					return;
				}
				if (rdTurnCnt.intTotal > 10)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeExceeded"]);
					return;
				}
				if (rdTurnCnt.intTotal <= 0)
				{
					dice_msg.Reply(GlobalMsg["strRollTimeErr"]);
					return;
				}
				intTurnCnt = rdTurnCnt.intTotal;
				if (strTurnCnt.find("d") != string::npos)
				{
					string strTurnNotice = strNickName + "的掷骰轮数: " + rdTurnCnt.FormShortString() + "轮";
					if (!isHidden)
					{
						dice_msg.Reply(strTurnNotice);
					}
					else
					{
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							strTurnNotice = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strTurnNotice;
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							strTurnNotice = "在多人聊天中 " + strTurnNotice;
						}
						AddMsgToQueue(Dice::DiceMsg(strTurnNotice, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
						pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							range = ObserveGroup.equal_range(dice_msg.group_id);
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							range = ObserveDiscuss.equal_range(dice_msg.group_id);
						}
						for (auto it = range.first; it != range.second; ++it)
						{
							if (it->second != dice_msg.qq_id)
							{
								AddMsgToQueue(Dice::DiceMsg(strTurnNotice, 0LL, it->second, Dice::MsgType::Private));
							}
						}
					}
				}
			}
			RD rdMainDice(strMainDice, dice_msg.qq_id);

			const int intFirstTimeRes = rdMainDice.Roll();
			if (intFirstTimeRes == Value_Err)
			{
				dice_msg.Reply(GlobalMsg["strValueErr"]);
				return;
			}
			if (intFirstTimeRes == Input_Err)
			{
				dice_msg.Reply(GlobalMsg["strInputErr"]);
				return;
			}
			if (intFirstTimeRes == ZeroDice_Err)
			{
				dice_msg.Reply(GlobalMsg["strZeroDiceErr"]);
				return;
			}
			if (intFirstTimeRes == ZeroType_Err)
			{
				dice_msg.Reply(GlobalMsg["strZeroTypeErr"]);
				return;
			}
			if (intFirstTimeRes == DiceTooBig_Err)
			{
				dice_msg.Reply(GlobalMsg["strDiceTooBigErr"]);
				return;
			}
			if (intFirstTimeRes == TypeTooBig_Err)
			{
				dice_msg.Reply(GlobalMsg["strTypeTooBigErr"]);
				return;
			}
			if (intFirstTimeRes == AddDiceVal_Err)
			{
				dice_msg.Reply(GlobalMsg["strAddDiceValErr"]);
				return;
			}
			if (intFirstTimeRes != 0)
			{
				dice_msg.Reply(GlobalMsg["strUnknownErr"]);
				return;
			}
			if (!boolDetail && intTurnCnt != 1)
			{
				string strAns = strNickName + "骰出了: " + to_string(intTurnCnt) + "次" + rdMainDice.strDice + ": { ";
				if (!strReason.empty())
					strAns.insert(0, "由于" + strReason + " ");
				vector<int> vintExVal;
				while (intTurnCnt--)
				{
					// 此处返回值无用
					// ReSharper disable once CppExpressionWithoutSideEffects
					rdMainDice.Roll();
					strAns += to_string(rdMainDice.intTotal);
					if (intTurnCnt != 0)
						strAns += ",";
					if ((rdMainDice.strDice == "D100" || rdMainDice.strDice == "1D100") && (rdMainDice.intTotal <= 5 ||
						rdMainDice.intTotal >= 96))
						vintExVal.push_back(rdMainDice.intTotal);
				}
				strAns += " }";
				if (!vintExVal.empty())
				{
					strAns += ",极值: ";
					for (auto it = vintExVal.cbegin(); it != vintExVal.cend(); ++it)
					{
						strAns += to_string(*it);
						if (it != vintExVal.cend() - 1)
							strAns += ",";
					}
				}
				if (!isHidden)
				{
					dice_msg.Reply(strAns);
				}
				else
				{
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						strAns = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strAns;
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						strAns = "在多人聊天中 " + strAns;
					}
					AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
					pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;					
					if (dice_msg.msg_type == Dice::MsgType::Group)
					{
						range = ObserveGroup.equal_range(dice_msg.group_id);
					}
					else if (dice_msg.msg_type == Dice::MsgType::Discuss)
					{
						range = ObserveDiscuss.equal_range(dice_msg.group_id);
					}
					for (auto it = range.first; it != range.second; ++it)
					{
						if (it->second != dice_msg.qq_id)
						{
							AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, it->second, Dice::MsgType::Private));
						}
					}
				}
			}
			else
			{
				while (intTurnCnt--)
				{
					// 此处返回值无用
					// ReSharper disable once CppExpressionWithoutSideEffects
					rdMainDice.Roll();
					string strAns = strNickName + "骰出了: " + (boolDetail
						? rdMainDice.FormCompleteString()
						: rdMainDice.FormShortString());
					if (!strReason.empty())
						strAns.insert(0, "由于" + strReason + " ");
					if (!isHidden)
					{
						dice_msg.Reply(strAns);
					}
					else
					{
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							strAns = "在群\"" + getGroupList()[dice_msg.group_id] + "\"中 " + strAns;
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							strAns = "在多人聊天中 " + strAns;
						}
						AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, dice_msg.qq_id, Dice::MsgType::Private));
						pair<multimap<long long, long long>::iterator, multimap<long long, long long>::iterator> range;
						if (dice_msg.msg_type == Dice::MsgType::Group)
						{
							range = ObserveGroup.equal_range(dice_msg.group_id);
						}
						else if (dice_msg.msg_type == Dice::MsgType::Discuss)
						{
							range = ObserveDiscuss.equal_range(dice_msg.group_id);
						}
						for (auto it = range.first; it != range.second; ++it)
						{
							if (it->second != dice_msg.qq_id)
							{
								AddMsgToQueue(Dice::DiceMsg(strAns, 0LL, it->second, Dice::MsgType::Private));
							}
						}
					}
				}
			}
			if (isHidden)
			{
				const string strReply = strNickName + "进行了一次暗骰";
				dice_msg.Reply(strReply);
			}
					else if (strLowerMessage.substr(intMsgCnt, 3) == "cat")//随机猫的分支
		{
			if (dice_msg.msg_type == Dice::MsgType::Private)
			{
				dice_msg.Reply(GlobalMsg["strCommandNotAvailableErr"]);
				return;
			}
			else
			{
				HANDLE CathThread = CreateThread(NULL, 0, CatImage, NULL, 0, NULL);/*下载Cat图片*/
				dice_msg.Reply(strNickName + "召唤了这样一只猫猫！");
				dice_msg.Reply("[CQ:image,file=cat.jpg]");
			}
			return;
		}
		else if (strLowerMessage.substr(intMsgCnt, 3) == "lmt")
		{
			intMsgCnt += 3;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string Command;
			while (intMsgCnt != strLowerMessage.length() && !isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])) && !isspace(
				static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
			{
				Command += strLowerMessage[intMsgCnt];
				intMsgCnt++;
			}
			if (Command == "on")
			{
				string strReply = LeaveGroupScanControl(true, dice_msg.qq_id);
				dice_msg.Reply(strReply);
			}
			else if (Command == "off")
			{
				string strReply = LeaveGroupScanControl(false, dice_msg.qq_id);
				dice_msg.Reply(strReply);
			}
			else if (Command == "add")
			{
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strGroupID;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strGroupID += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (strGroupID.empty())
				{
					dice_msg.Reply("你呼叫的群号是空号！请查证后再拨！");
					return;
				}
				const long long llGroupID = stoll(strGroupID);
				string strReply = WhiteListControl(dice_msg.qq_id, llGroupID, true);
				dice_msg.Reply(strReply);
			}
			else if (Command == "rmv")
			{
				while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
					intMsgCnt++;
				string strGroupID;
				while (isdigit(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				{
					strGroupID += strLowerMessage[intMsgCnt];
					intMsgCnt++;
				}
				if (strGroupID.empty())
				{
					dice_msg.Reply("你呼叫的群号是空号！请查证后再拨！");
					return;
				}
				const long long llGroupID = stoll(strGroupID);
				string strReply = WhiteListControl(dice_msg.qq_id, llGroupID, false);
				dice_msg.Reply(strReply);
			}
			else if (Command == "repo")
			{
					string ListReport = LMTRecReport(dice_msg.qq_id);
					dice_msg.Reply(ListReport);
			}
			else if (Command == "refr")
			{
				string RefrRepo = RefrList(dice_msg.qq_id).LMTMsg;
				dice_msg.Reply(RefrRepo);
			}
			else if (Command == "check")
			{
				if (dice_msg.qq_id != MasterQQID)
				{
					dice_msg.Reply("你不是我的老板，我才不听你的");
					return;
				}
				map<long long, string> GroupList = CQ::getGroupList();
				dice_msg.Reply("当前群列表里有" + to_string(GroupList.size()) + "个群,请核对正确后再启动lts哟！");
			}
			else if (Command == "pack")
			{
				if (dice_msg.qq_id != MasterQQID)
				{
					dice_msg.Reply("你不是我的老板，我才不听你的");
					return;
				}
				ListPack(true, false);
				dice_msg.Reply("已备份闲置监视的数据！");
			}
			else
			{
				return;
			}
			return;
		}
		else if (strLowerMessage.substr(intMsgCnt, 8) == "tomaster")
		{
			intMsgCnt += 8;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			string strAction = strip(dice_msg.msg.substr(intMsgCnt));
			if (strAction.length() >= 1)
			{
				dice_msg.Reply("好的，我这就告诉柴刀");
				map<long long, string> GroupList = CQ::getGroupList();
				strAction = "来自群" + to_string(dice_msg.group_id) + "(" + GroupList[dice_msg.group_id] + ")的" + to_string(dice_msg.qq_id) + "(" + getGroupMemberInfo(dice_msg.group_id, dice_msg.qq_id).GroupNick + ")的消息\n" + strAction;
				CQ::sendPrivateMsg(MasterQQID, strAction);
			}
			else
			{
				dice_msg.Reply("我不知道你想说什么");
			}
			return;
		}
		else if (strLowerMessage.substr(intMsgCnt, 2) == "cs")
		{
			intMsgCnt += 2;
			while (isspace(static_cast<unsigned char>(strLowerMessage[intMsgCnt])))
				intMsgCnt++;
			if (intMsgCnt == strLowerMessage.length())/*.cs后面什么都没有*/
			{
				dice_msg.Reply("根据记录，" + strNickName + "在青木莲这里的所有的检定的结果总结如下：所有（近一周）" + CheckSumReporter(dice_msg.qq_id));
				return;
			}
			if (strLowerMessage.substr(intMsgCnt, 3) == "clr")
			{
				if (CheckSumMap.count(dice_msg.qq_id))
					CheckSumMap.erase(dice_msg.qq_id);
				dice_msg.Reply("按照要求" + strNickName + "在青木莲这里的所有的检定的结果已被清除");
			}
		}
		}
		else
		{
			block_msg = false;
		}
	}
	void EventHandler::HandleGroupMemberIncreaseEvent(long long beingOperateQQ, long long fromGroup)
	{
		if (beingOperateQQ != getLoginQQ() && WelcomeMsg.count(fromGroup))
		{
			string strReply = WelcomeMsg[fromGroup];
			while (strReply.find("{@}") != string::npos)
			{
				strReply.replace(strReply.find("{@}"), 3, "[CQ:at,qq=" + to_string(beingOperateQQ) + "]");
			}
			while (strReply.find("{nick}") != string::npos)
			{
				strReply.replace(strReply.find("{nick}"), 6, getStrangerInfo(beingOperateQQ).nick);
			}
			while (strReply.find("{age}") != string::npos)
			{
				strReply.replace(strReply.find("{age}"), 5, to_string(getStrangerInfo(beingOperateQQ).age));
			}
			while (strReply.find("{sex}") != string::npos)
			{
				strReply.replace(strReply.find("{sex}"), 5,
					getStrangerInfo(beingOperateQQ).sex == 0
					? "男"
					: getStrangerInfo(beingOperateQQ).sex == 1
					? "女"
					: "未知");
			}
			while (strReply.find("{qq}") != string::npos)
			{
				strReply.replace(strReply.find("{qq}"), 4, to_string(beingOperateQQ));
			}
			AddMsgToQueue(Dice::DiceMsg(std::move(strReply), fromGroup, fromGroup, Dice::MsgType::Group));
		}
	}
	void EventHandler::HandleDisableEvent()
	{
		Enabled = false;
		ilInitList.reset();
		Name.reset();
		ofstream ofstreamDisabledGroup(strFileLoc + "DisabledGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledGroup.begin(); it != DisabledGroup.end(); ++it)
		{
			ofstreamDisabledGroup << *it << std::endl;
		}
		ofstreamDisabledGroup.close();

		ofstream ofstreamDisabledDiscuss(strFileLoc + "DisabledDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledDiscuss.begin(); it != DisabledDiscuss.end(); ++it)
		{
			ofstreamDisabledDiscuss << *it << std::endl;
		}
		ofstreamDisabledDiscuss.close();
		ofstream ofstreamDisabledJRRPGroup(strFileLoc + "DisabledJRRPGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledJRRPGroup.begin(); it != DisabledJRRPGroup.end(); ++it)
		{
			ofstreamDisabledJRRPGroup << *it << std::endl;
		}
		ofstreamDisabledJRRPGroup.close();

		ofstream ofstreamDisabledJRRPDiscuss(strFileLoc + "DisabledJRRPDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledJRRPDiscuss.begin(); it != DisabledJRRPDiscuss.end(); ++it)
		{
			ofstreamDisabledJRRPDiscuss << *it << std::endl;
		}
		ofstreamDisabledJRRPDiscuss.close();

		ofstream ofstreamDisabledHELPGroup(strFileLoc + "DisabledHELPGroup.RDconf", ios::in | ios::trunc);
		for (auto it = DisabledHELPGroup.begin(); it != DisabledHELPGroup.end(); ++it)
		{
			ofstreamDisabledHELPGroup << *it << std::endl;
		}
		ofstreamDisabledHELPGroup.close();

		ofstream ofstreamDisabledHELPDiscuss(strFileLoc + "DisabledHELPDiscuss.RDconf", ios::in | ios::trunc);
		for (auto it = DisabledHELPDiscuss.begin(); it != DisabledHELPDiscuss.end(); ++it)
		{
			ofstreamDisabledHELPDiscuss << *it << std::endl;
		}
		ofstreamDisabledHELPDiscuss.close();

		ofstream ofstreamDisabledOBGroup(strFileLoc + "DisabledOBGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledOBGroup.begin(); it != DisabledOBGroup.end(); ++it)
		{
			ofstreamDisabledOBGroup << *it << std::endl;
		}
		ofstreamDisabledOBGroup.close();

		ofstream ofstreamDisabledOBDiscuss(strFileLoc + "DisabledOBDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledOBDiscuss.begin(); it != DisabledOBDiscuss.end(); ++it)
		{
			ofstreamDisabledOBDiscuss << *it << std::endl;
		}
		ofstreamDisabledOBDiscuss.close();

		ofstream ofstreamObserveGroup(strFileLoc + "ObserveGroup.RDconf", ios::out | ios::trunc);
		for (auto it = ObserveGroup.begin(); it != ObserveGroup.end(); ++it)
		{
			ofstreamObserveGroup << it->first << " " << it->second << std::endl;
		}
		ofstreamObserveGroup.close();

		ofstream ofstreamObserveDiscuss(strFileLoc + "ObserveDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = ObserveDiscuss.begin(); it != ObserveDiscuss.end(); ++it)
		{
			ofstreamObserveDiscuss << it->first << " " << it->second << std::endl;
		}
		ofstreamObserveDiscuss.close();
		ofstream ofstreamCharacterProp(strFileLoc + "CharacterProp.RDconf", ios::out | ios::trunc);
		for (auto it = CharacterProp.begin(); it != CharacterProp.end(); ++it)
		{
			for (auto it1 = it->second.cbegin(); it1 != it->second.cend(); ++it1)
			{
				ofstreamCharacterProp << it->first.QQ << " " << it->first.Type << " " << it->first.GrouporDiscussID << " "
					<< it1->first << " " << it1->second << std::endl;
			}
		}
		ofstreamCharacterProp.close();
		ofstream ofstreamDefault(strFileLoc + "Default.RDconf", ios::out | ios::trunc);
		for (auto it = DefaultDice.begin(); it != DefaultDice.end(); ++it)
		{
			ofstreamDefault << it->first << " " << it->second << std::endl;
		}
		ofstreamDefault.close();

		ofstream ofstreamWelcomeMsg(strFileLoc + "WelcomeMsg.RDconf", ios::out | ios::trunc);
		for (auto it = WelcomeMsg.begin(); it != WelcomeMsg.end(); ++it)
		{
			while (it->second.find(' ') != string::npos)it->second.replace(it->second.find(' '), 1, "{space}");
			while (it->second.find('\t') != string::npos)it->second.replace(it->second.find('\t'), 1, "{tab}");
			while (it->second.find('\n') != string::npos)it->second.replace(it->second.find('\n'), 1, "{endl}");
			while (it->second.find('\r') != string::npos)it->second.replace(it->second.find('\r'), 1, "{enter}");
			ofstreamWelcomeMsg << it->first << " " << it->second << std::endl;
		}
		ofstreamWelcomeMsg.close();
		
		ofstream ofstreamRoomRule(strFileLoc + "RoomRule.RDconf", ios::out | ios::trunc);
		for (auto it = RoomRule.begin(); it != RoomRule.end(); ++it)
		{
			ofstreamRoomRule << it->first << " " << it->second << std::endl;
		}
		ofstreamRoomRule.close();

		DefaultDice.clear();
		DisabledGroup.clear();
		DisabledDiscuss.clear();
		DisabledJRRPGroup.clear();
		DisabledJRRPDiscuss.clear();
		DisabledOBGroup.clear();
		DisabledOBDiscuss.clear();
		ObserveGroup.clear();
		ObserveDiscuss.clear();
		strFileLoc.clear();
		RoomRule.clear();
	}
	void EventHandler::HandleExitEvent()
	{
		if (!Enabled) return;
		ilInitList.reset();
		Name.reset();
		ofstream ofstreamDisabledGroup(strFileLoc + "DisabledGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledGroup.begin(); it != DisabledGroup.end(); ++it)
		{
			ofstreamDisabledGroup << *it << std::endl;
		}
		ofstreamDisabledGroup.close();

		ofstream ofstreamDisabledDiscuss(strFileLoc + "DisabledDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledDiscuss.begin(); it != DisabledDiscuss.end(); ++it)
		{
			ofstreamDisabledDiscuss << *it << std::endl;
		}
		ofstreamDisabledDiscuss.close();
		ofstream ofstreamDisabledJRRPGroup(strFileLoc + "DisabledJRRPGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledJRRPGroup.begin(); it != DisabledJRRPGroup.end(); ++it)
		{
			ofstreamDisabledJRRPGroup << *it << std::endl;
		}
		ofstreamDisabledJRRPGroup.close();

		ofstream ofstreamDisabledJRRPDiscuss(strFileLoc + "DisabledJRRPDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledJRRPDiscuss.begin(); it != DisabledJRRPDiscuss.end(); ++it)
		{
			ofstreamDisabledJRRPDiscuss << *it << std::endl;
		}
		ofstreamDisabledJRRPDiscuss.close();

		ofstream ofstreamDisabledHELPGroup(strFileLoc + "DisabledHELPGroup.RDconf", ios::in | ios::trunc);
		for (auto it = DisabledHELPGroup.begin(); it != DisabledHELPGroup.end(); ++it)
		{
			ofstreamDisabledHELPGroup << *it << std::endl;
		}
		ofstreamDisabledHELPGroup.close();

		ofstream ofstreamDisabledHELPDiscuss(strFileLoc + "DisabledHELPDiscuss.RDconf", ios::in | ios::trunc);
		for (auto it = DisabledHELPDiscuss.begin(); it != DisabledHELPDiscuss.end(); ++it)
		{
			ofstreamDisabledHELPDiscuss << *it << std::endl;
		}
		ofstreamDisabledHELPDiscuss.close();

		ofstream ofstreamDisabledOBGroup(strFileLoc + "DisabledOBGroup.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledOBGroup.begin(); it != DisabledOBGroup.end(); ++it)
		{
			ofstreamDisabledOBGroup << *it << std::endl;
		}
		ofstreamDisabledOBGroup.close();

		ofstream ofstreamDisabledOBDiscuss(strFileLoc + "DisabledOBDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = DisabledOBDiscuss.begin(); it != DisabledOBDiscuss.end(); ++it)
		{
			ofstreamDisabledOBDiscuss << *it << std::endl;
		}
		ofstreamDisabledOBDiscuss.close();

		ofstream ofstreamObserveGroup(strFileLoc + "ObserveGroup.RDconf", ios::out | ios::trunc);
		for (auto it = ObserveGroup.begin(); it != ObserveGroup.end(); ++it)
		{
			ofstreamObserveGroup << it->first << " " << it->second << std::endl;
		}
		ofstreamObserveGroup.close();

		ofstream ofstreamObserveDiscuss(strFileLoc + "ObserveDiscuss.RDconf", ios::out | ios::trunc);
		for (auto it = ObserveDiscuss.begin(); it != ObserveDiscuss.end(); ++it)
		{
			ofstreamObserveDiscuss << it->first << " " << it->second << std::endl;
		}
		ofstreamObserveDiscuss.close();
		ofstream ofstreamCharacterProp(strFileLoc + "CharacterProp.RDconf", ios::out | ios::trunc);
		for (auto it = CharacterProp.begin(); it != CharacterProp.end(); ++it)
		{
			for (auto it1 = it->second.cbegin(); it1 != it->second.cend(); ++it1)
			{
				ofstreamCharacterProp << it->first.QQ << " " << it->first.Type << " " << it->first.GrouporDiscussID << " "
					<< it1->first << " " << it1->second << std::endl;
			}
		}
		ofstreamCharacterProp.close();
		ofstream ofstreamDefault(strFileLoc + "Default.RDconf", ios::out | ios::trunc);
		for (auto it = DefaultDice.begin(); it != DefaultDice.end(); ++it)
		{
			ofstreamDefault << it->first << " " << it->second << std::endl;
		}
		ofstreamDefault.close();

		ofstream ofstreamWelcomeMsg(strFileLoc + "WelcomeMsg.RDconf", ios::out | ios::trunc);
		for (auto it = WelcomeMsg.begin(); it != WelcomeMsg.end(); ++it)
		{
			while (it->second.find(' ') != string::npos)it->second.replace(it->second.find(' '), 1, "{space}");
			while (it->second.find('\t') != string::npos)it->second.replace(it->second.find('\t'), 1, "{tab}");
			while (it->second.find('\n') != string::npos)it->second.replace(it->second.find('\n'), 1, "{endl}");
			while (it->second.find('\r') != string::npos)it->second.replace(it->second.find('\r'), 1, "{enter}");
			ofstreamWelcomeMsg << it->first << " " << it->second << std::endl;
		}
		ofstreamWelcomeMsg.close();
		
		ofstream ofstreamRoomRule(strFileLoc + "RoomRule.RDconf", ios::out | ios::trunc);
		for (auto it = RoomRule.begin(); it != RoomRule.end(); ++it)
		{
			ofstreamRoomRule << it->first << " " << it->second << std::endl;
		}
		ofstreamRoomRule.close();
	}
}


