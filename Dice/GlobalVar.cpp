/*
 *  _______     ________    ________    ________    __
 * |   __  \   |__    __|  |   _____|  |   _____|  |  |
 * |  |  |  |     |  |     |  |        |  |_____   |  |
 * |  |  |  |     |  |     |  |        |   _____|  |__|
 * |  |__|  |   __|  |__   |  |_____   |  |_____    __
 * |_______/   |________|  |________|  |________|  |__|
 *
 * Dice! QQ Dice Robot for TRPG
 * Copyright (C) 2018-2019 w4123溯洄
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with this
 * program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "CQLogger.h"
#include "GlobalVar.h"
#include <map>

bool Enabled = false;

bool msgSendThreadRunning = false;

CQ::logger DiceLogger("Dice!");

/*
 * 版本信息
 * 请勿修改Dice_Build, Dice_Ver_Without_Build，DiceRequestHeader以及Dice_Ver常量
 * 请修改Dice_Short_Ver或Dice_Full_Ver常量以达到版本自定义
 */
const unsigned short Dice_Build = 524;
const std::string Dice_Ver_Without_Build = "2.3.8";
const std::string DiceRequestHeader = "Dice/" + Dice_Ver_Without_Build;
const std::string Dice_Ver = Dice_Ver_Without_Build + "(" + std::to_string(Dice_Build) + ")";
const std::string Dice_Short_Ver = "Dice! by 溯洄_多米诺魔改 Ver. " + Dice_Ver;
#ifdef __clang__

#ifdef _MSC_VER
const std::string Dice_Full_Ver = Dice_Short_Ver + " [CLANG " + __clang_version__ + " MSVC " + std::to_string(_MSC_FULL_VER) + " " + __DATE__ + " " + __TIME__ + "]";
#elif defined(__GNUC__)
const std::string Dice_Full_Ver = Dice_Short_Ver + " [CLANG " + __clang_version__ + " GNUC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__) + " " + __DATE__ + " " + __TIME__ + "]";
#else
const std::string Dice_Full_Ver = Dice_Short_Ver + " [CLANG " + __clang_version__ + " UNKNOWN]"
#endif /*_MSC_VER*/

#else

#ifdef _MSC_VER
const std::string Dice_Full_Ver = Dice_Short_Ver/* + " [MSVC " + std::to_string(_MSC_FULL_VER) + " " + __DATE__ + " " + __TIME__ + "]"*/;
#elif defined(__GNUC__)
const std::string Dice_Full_Ver = Dice_Short_Ver + " [GNUC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__) + " " + __DATE__ + " " + __TIME__ + "]";
#else
const std::string Dice_Full_Ver = Dice_Short_Ver + " [UNKNOWN COMPILER]"
#endif /*_MSC_VER*/

#endif /*__clang__*/

std::map<const std::string, std::string> GlobalMsg
{
	{"strNameNumTooBig", "你要本大爷生成那么多名字干嘛！\n请输入1-10之间的数字！"},
	{"strNameNumCannotBeZero", "本大爷给你变个魔术喔！\n→        ←\n你看到了吗？这就是本大爷给你取的名字！"},
	{"strSetInvalid", "你这是什么默认骰哇！！\n请输入1-99999之间的数字嗷！"},
	{"strSetTooBig", "这么多面的默认骰？！\n你真的想让本大爷丢个球嘛！"},
	{"strSetCannotBeZero", "“你这……”\n682思考着，试图找出一个0面的骰子。"},
	{"strCharacterCannotBeZero", "“诶？！”\n682为你生成了一个不存在的东西。"},
	{"strCharacterTooBig", "你要生成那么多人干什么啦！养孩子玩嘛？！！"},
	{"strCharacterInvalid", "人物作成次数无效嗷！\n请输入1-10之间的数字！"},
	{"strSCInvalid", "你到底会不会Sancheck哇！\n格式为成功扣San/失败扣San,如1/1d6！"},
	{"strSanInvalid", "你这个San值是怎么回事啦！\n请输入1-99范围内的整数！"},
	{"strEnValInvalid", "你这是个什么属性啦！\n请输入1-99范围内的整数！"},
	{"strNameDelErr", "根本就没有设置名字吧！"},
	{"strValueErr", "你这发的啥呀……本大爷看不懂！(理直气壮)"},
	{"strInputErr", "命令或掷骰表达式输入错误!"},
	{"strUnknownErr", "你这发的啥呀……本大爷看不懂！(理直气壮)"},
	{"strMEDisabledErr", "好嗷！我把.me命令关掉啦——"},
	{"strGroupIDInvalid", "682翻了半天，没有找到这样的群"},
	{"strSendErr", "消息发送失败了！为什么会这样——"},
	{"strDisabledErr", "“可是，可是那边不让我说话呀……”(命令无法执行:我已在此群中被关闭!)"},
	{"strUnableToGetErrorMsg", "那个……本大爷也不知道哪里出问题了"},
	{"strDiceTooBigErr", "喂喂！等一下！\n那么多骰子本大爷根本数不过来啊！！"},
	{"strRequestRetCodeErr", "啊……你要查询的东西被土豆吃掉了！\n HTTP状态码: {0}"},
	{"strRequestNoResponse", "好像……服务器不理本大爷了，怎么回事？"},
	{"strTypeTooBigErr", "等下！你丢了个啥啊！它滚不见了！！\n你等着，本大爷去找找跑到哪里去了……"},
	{"strZeroTypeErr", "682看着面前不可名状的骰子停止了思考。"},
	{"strAddDiceValErr", "不要再加骰子了！！\n本大爷骰不完那么多骰子啊！！！"},
	{"strZeroDiceErr", "诶?本大爷的骰子到哪儿去了?"},
	{"strRollTimeExceeded", "本大爷可警告你，骰那么多轮会被管理员当刷屏打死的！"},
	{"strRollTimeErr", "诶？你这是要骰多少轮？？"},
	{"strWelcomeMsgClearedNotice", "本大爷已经把这个群的欢迎词给忘记啦！"},
	{"strWelcomeMsgIsEmptyErr", "你没告诉过我新的D级人员来了之后应该怎么说啊？"},
	{"strWelcomeMsgUpdatedNotice", "本大爷知道啦！\n以后有新的D级人员来了我就这么说——"},
	{"strPermissionDeniedErr", "嘿，你又不是管理员，本大爷不听你的！"},
	{"strNameTooLongErr", "你名字太长了！本大爷记不住！"},
	{"strUnknownPropErr", "你没告诉过我这个属性啊……"},
	{"strEmptyWWDiceErr", "你骰错格式了嗷！\n正确格式为.w(w)XaY！其中X≥1, 5≤Y≤10"},
	{"strPropErr", "给本大爷认真输入你的属性啊！！"},
	{"strSetPropSuccess", "本大爷记住你的属性啦！"},
	{"strPropCleared", "本大爷把你的所有属性都忘光啦！"},
	{"strRuleErr", "好像出现了什么错误！\n具体信息:\n"},
	{"strRulesFailedErr", "啊……好像服务器的土豆熟了呢，本大爷访问不上啊……"},
	{"strPropDeleted", "本大爷把这个属性忘掉啦！"},
	{"strPropNotFound", "……你没有告诉过本大爷你这个属性啊！"},
	{"strRuleNotFound", "……本大爷在规则书上查不到这个啊……"},
	{"strProp", "噗，{0}，你的{1}才只有{2}点诶，要不要本大爷帮你给骰子做做手脚？"},
	{"strStErr", "你的格式输错啦，笨蛋！\n找个大佬问问.st命令的使用方法吧！"},
	{"strRulesFormatErr", "你输错格式啦:正确格式为.rules[规则名称:]规则条目 如.rules COC:力量"},
	{"strJrrp", "{0}今天跑团可以骰出{1}个大成功！！"},
	{"strJrrpErr", "啊……你的JRRP好像被土豆吃掉了！\n错误信息: \n{0}"},
	{"strFumble", "诶嘿…大失败，这可是你自己骰的，跟本大爷没有关系！\n682与KP勾肩搭背，愉悦地晃着尾巴"},
	{"strFailure", "诶嘿，失败了！"},
	{"strSuccess", "成功了嗷！"},
	{"strHardSuccess", "噢吼，困难成功耶！"},
	{"strExtremeSuccess", "这是……极难成功嗷！！"},
	{"strSCFumble", "嗷，是大失败——"},
	{"strCriticalSuccess", "这是……大成功？！\n682偷偷拿起骰子，想要重新骰一次。"},
	{"strEnFailure", "噗，你在这个时候骰运又意外地好起来了……"},
	{"strEnSuccess", "噢吼，居然成功了！"},
	{"strSCFailure", "“失败了嗷！”"},
	{"strSCSuccess", "“成功了！！”"},
	{"strInitListClearedNotice", "成功清除先攻记录!"},
	{"strInitListIsEmptyErr", "错误: 先攻列表为空!"},
	{"strCommandNotAvailableErr", "这个命令只能在群或者讨论组中使用嗷"},
	{"strSuccessfullyEnabledNotice", "“嗷——？”\n682从收容室中探出半个脑袋（成功开启）"},
	{"strAlreadyEnabledErr", "“干啥？！”682烦躁地晃着尾巴\n(机器人处于开启状态)"},
	{"strSuccessfullyDisabledNotice", "“咕嘟咕嘟……”什么庞大的东西沉到了盐酸池底部（成功关闭）"},
	{"strAlreadyDisabledErr", "盐酸池底部冒出了一串泡泡\n(机器人处于关闭状态)"},
	{"strObCommandSuccessfullyEnabledNotice", "成功在本群/讨论组中启用旁观模式!"},
	{"strObCommandAlreadyEnabledErr", "错误: 在本群/讨论组旁观模式已经被启用!"},
	{"strObCommandSuccessfullyDisabledNotice", "成功在本群/讨论组中禁用旁观模式!"},
	{"strObCommandAlreadyDisabledErr", "错误: 在本群/讨论组旁观模式已经被禁用!"},
	{"strObCommandDisabledErr", "管理员已在本群/讨论组中禁用旁观模式!"},
	{"strObListClearedNotice", "成功删除所有旁观者!"},
	{"strJrrpCommandSuccessfullyEnabledNotice", "成功在本群/讨论组中启用.jrrp命令!"},
	{"strJrrpCommandAlreadyEnabledErr", "错误: 在本群/讨论组中.jrrp命令已经被启用!"},
	{"strJrrpCommandSuccessfullyDisabledNotice", "成功在本群/讨论组中禁用.jrrp命令!"},
	{"strJrrpCommandAlreadyDisabledErr", "错误: 在本群/讨论组中.jrrp命令已经被禁用!"},
	{"strJrrpCommandDisabledErr", "管理员已在此群/讨论组中禁用.jrrp命令!"},
	{"strHelpCommandSuccessfullyEnabledNotice", "成功在本群/讨论组中启用.help命令!"},
	{"strHelpCommandAlreadyEnabledErr", "错误: 在本群/讨论组中.help命令已经被启用!"},
	{"strHelpCommandSuccessfullyDisabledNotice", "成功在本群/讨论组中禁用.help命令!"},
	{"strHelpCommandAlreadyDisabledErr", "错误: 在本群/讨论组中.help命令已经被禁用!"},
	{"strHelpCommandDisabledErr", "管理员已在此群/讨论组中禁用.help命令!"},
	{"strRoomRuleSetErr","大成功大失败概率无效，请输入0-50内的整数嗷！"},
	{"strRoomRuleClear","已将房规还原为默认值了嗷！（大成功1-5，大失败96-100）"},
	{"strRoomRuleClearErr","现在没有设置房规嗷——"},
	{"strRoomRuleSet","房规设置成功了嗷！"},
	{"strHelpMsg" , Dice_Short_Ver + "\n" + "请使用!dismiss 2602617724命令让682自动退群喔！\n禁言和踢群会被拉黑，请务必注意——\n更多信息请扫描二维码——[CQ:image,file=HELP.jpg]"}
};