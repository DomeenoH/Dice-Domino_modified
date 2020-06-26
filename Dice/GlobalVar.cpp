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
	{"strNameNumTooBig", "生成数量过多!请输入1-10之间的数字!"},
	{"strNameNumCannotBeZero", "生成数量不能为零!请输入1-10之间的数字!"},
	{"strSetInvalid", "无效的默认骰!请输入1-99999之间的数字!"},
	{"strSetTooBig", "默认骰过大!请输入1-99999之间的数字!"},
	{"strSetCannotBeZero", "默认骰不能为零!请输入1-99999之间的数字!"},
	{"strCharacterCannotBeZero", "人物作成次数不能为零!请输入1-10之间的数字!"},
	{"strCharacterTooBig", "人物作成次数过多!请输入1-10之间的数字!"},
	{"strCharacterInvalid", "人物作成次数无效!请输入1-10之间的数字!"},
	{"strSCInvalid", "SC表达式输入不正确,格式为成功扣San/失败扣San,如1/1d6!"},
	{"strSanInvalid", "San值输入不正确,请输入1-99范围内的整数!"},
	{"strEnValInvalid", "技能值或属性输入不正确,请输入1-99范围内的整数!"},
	{"strNameDelErr", "没有设置名称,无法删除!"},
	{"strValueErr", "掷骰表达式输入错误!"},
	{"strInputErr", "命令或掷骰表达式输入错误!"},
	{"strUnknownErr", "发生了未知错误!"},
	{"strMEDisabledErr", "已禁用.me命令!过家家禁止x"},
	{"strGroupIDInvalid", "查无此群!请查证后再拨!"},
	{"strSendErr", "消息发送失败！邮件被邮差吃掉了！"},
	{"strDisabledErr", "不让咱说话还想让咱干活?(命令无法执行:青木莲已在此群中被关闭!)"},
	{"strUnableToGetErrorMsg", "无法获取错误信息!"},
	{"strDiceTooBigErr", "骰娘被你扔出的骰子淹没了"},
	{"strRequestRetCodeErr", "访问服务器时出现错误! HTTP状态码: {0}"},
	{"strRequestNoResponse", "服务器未返回任何信息"},
	{"strTypeTooBigErr", "哇!让我数数骰子有多少面先~1...2..."},
	{"strZeroTypeErr", "这是...!!时空裂(骰娘被骰子产生的时空裂缝卷走了)"},
	{"strAddDiceValErr", "你这样要让我扔骰子扔到什么时候嘛~(请输入正确的加骰参数:5-10之内的整数)"},
	{"strZeroDiceErr", "咦?我的骰子呢?"},
	{"strRollTimeExceeded", "掷骰轮数超过了最大轮数限制!"},
	{"strRollTimeErr", "异常的掷骰轮数"},
	{"strWelcomeMsgClearedNotice", "已清除本群的入群欢迎词"},
	{"strWelcomeMsgIsEmptyErr", "错误:没有设置入群欢迎词，清除失败"},
	{"strWelcomeMsgUpdatedNotice", "已更新本群的入群欢迎词"},
	{"strPermissionDeniedErr", "错误:此操作需要群主或管理员权限"},
	{"strNameTooLongErr", "错误:名称过长(最多为50英文字符)"},
	{"strUnknownPropErr", "错误:属性不存在"},
	{"strEmptyWWDiceErr", "格式错误:正确格式为.w(w)XaY!其中X≥1, 5≤Y≤10"},
	{"strPropErr", "请认真的输入你的属性哦~"},
	{"strSetPropSuccess", "属性设置成功"},
	{"strPropCleared", "已清除所有属性"},
	{"strRuleErr", "规则数据获取失败,具体信息:\n"},
	{"strRulesFailedErr", "请求失败,无法连接数据库"},
	{"strPropDeleted", "属性删除成功"},
	{"strPropNotFound", "错误:属性不存在"},
	{"strRuleNotFound", "未找到对应的规则信息"},
	{"strProp", "{0}的{1}属性值为{2}"},
	{"strStErr", "格式错误:请参考帮助文档获取.st命令的使用方法"},
	{"strRulesFormatErr", "格式错误:正确格式为.rules[规则名称:]规则条目 如.rules COC7:力量"},
	{"strJrrp", "{0}今天的人品值是: {1}"},
	{"strJrrpErr", "JRRP获取失败! 错误信息: \n{0}"},
	{"strFumble", "大失败"},
	{"strFailure", "失败"},
	{"strSuccess", "成功"},
	{"strHardSuccess", "困难成功"},
	{"strExtremeSuccess", "极难成功"},
	{"strSCFumble", "哎呀呀，是大失败..."},
	{"strCriticalSuccess", "大成功"},
	{"strSCFailure", "嗯...失败了呢"},
	{"strSCSuccess", "嘁，成功了"},
	{"strENFailure", "失败了，你一无所获"},
	{"strENSuccess", "成功了哦"},
	{"strInitListClearedNotice", "成功清除先攻记录!"},
	{"strInitListIsEmptyErr", "错误: 先攻列表为空!"},
	{"strCommandNotAvailableErr", "错误: 此命令仅可在群/讨论组中使用"},
	{"strSuccessfullyEnabledNotice", "成功开启本机器人!"},
	{"strAlreadyEnabledErr", "本机器人已经处于开启状态!"},
	{"strSuccessfullyDisabledNotice", "成功关闭本机器人!"},
	{"strAlreadyDisabledErr", "本机器人已经处于关闭状态!"},
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
	{"strRoomRuleSetErr","大成功大失败概率无效，请输入0-50内的整数！"},
	{"strRoomRuleClear","已将房规还原为默认值（大成功1-5，大失败96-100）"},
	{"strRoomRuleClearErr","未设置房规，无需还原"},
	{"strRoomRuleSet","设置房规成功！"},
	{"strHelpMsg" , Dice_Short_Ver + "\n" + "更多信息请扫描二维码——[CQ:image,file=HELP.jpg]"}
};