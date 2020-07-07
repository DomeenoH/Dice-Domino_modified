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
	{"strNameNumTooBig", "噫，多多一时半会儿想不出那么多名字呀!最多最多10个!"},
	{"strNameNumCannotBeZero", "喵……？你到底要不要我起名字？两脚兽真奇怪"},
	{"strSetInvalid", "多多这里没有这么奇怪的骰子!请输入1-99999之间的数字!"},
	{"strSetTooBig", "这么多面……这是一个球吧!请输入1-99999之间的数字!"},
	{"strSetCannotBeZero", "0面骰是个什么奇怪的骰子啦!请输入1-99999之间的数字!"},
	{"strCharacterCannotBeZero", "喵……？你到底要不要我帮忙生成属性？两脚兽真奇怪"},
	{"strCharacterTooBig", "“太多啦太多啦!”多多手忙脚乱地记录着骰子出目，“不行不行，最多一次10个，不能再多了！！”"},
	{"strCharacterInvalid", "多多没搞懂你的指令，试试.coc3？"},
	{"strSCInvalid", "“唔……你的sc指令输错啦，”多多试着向你解释，“格式为成功扣San/失败扣San,如果没有记录san值的话，后面也要加上。输入1/1d6 50试试？”"},
	{"strSanInvalid", "你的san值好奇怪诶,人类的理智值范围是1-99内的整数喔!"},
	{"strEnValInvalid", "你的技能值好奇怪诶,输入1-99范围内的整数试试？"},
	{"strNameDelErr", "笨蛋两脚兽，你根本就没有告诉我你的名字吧！"},
	{"strValueErr", "多多看着你的指令愣了一小会儿，但还是没能理解这到底是什么意思"},
	{"strInputErr", "多多看着你的指令愣了一小会儿，但还是没能理解这到底是什么意思"},
	{"strUnknownErr", "“好像哪里出错了……可是到底是哪里啊？？”多多攥着一把骰子不知所措"},
	{"strMEDisabledErr", "好喔！多多把.me命令关掉啦——"},
	{"strGroupIDInvalid", "多多翻了半天，没有找到这样的群"},
	{"strSendErr", "消息发送失败了！为什么会这样——"},
	{"strDisabledErr", "“可是，可是那边不让我说话呀……”(命令无法执行:多多已在此群中被关闭!)"},
	{"strUnableToGetErrorMsg", "无法获取错误信息!"},
	{"strDiceTooBigErr", "“等一下，等……！！”多多手忙脚乱地抱着一大把骰子，急得快要哭出来，“这么多骰子我根本数不过来哇！”"},
	{"strRequestRetCodeErr", "访问服务器时好像出错了，具体问题的话，可以百度一下HTTP状态码: {0}"},
	{"strRequestNoResponse", "服务器未返回任何信息"},
	{"strTypeTooBigErr", "这……这骰子圆滚滚的根本看不出来出目啊！"},
	{"strZeroTypeErr", "“这个骰子好奇怪……”多多看着手里不可名状的骰子神情恍惚。"},
	{"strAddDiceValErr", "你这样要让我扔骰子扔到什么时候嘛！(请输入正确的加骰参数:5-10之内的整数)"},
	{"strZeroDiceErr", "“我的骰子到哪里去了？是在这里吗——”多多钻进角落里，只露出摇晃着的尾巴"},
	{"strRollTimeExceeded", "太多了太多了！！"},
	{"strRollTimeErr", "异常的掷骰轮数"},
	{"strWelcomeMsgClearedNotice", "嘛，以后有新俘虏上船了你们就自己处理吧！"},
	{"strWelcomeMsgIsEmptyErr", "本来你就没有让我去管新上船的俘虏嘛！"},
	{"strWelcomeMsgUpdatedNotice", "知道啦！以后有新的俘虏被捉上船了，我就这么告诉他，嘿嘿……"},
	{"strPermissionDeniedErr", "等什么时候你成了我的大副，或者打败我成为新的船长，再来对我发号施令吧！"},
	{"strNameTooLongErr", "你这是哪国的名字……？好长啊我记不住！"},
	{"strUnknownPropErr", "你有告诉过我这个属性吗……？可能被我忘掉了哈哈哈"},
	{"strEmptyWWDiceErr", "格式错误:正确格式为.w(w)XaY!其中X≥1, 5≤Y≤10"},
	{"strPropErr", "错啦错啦，属性不是这样输入的！你再检查一下是不是哪里写错了——"},
	{"strSetPropSuccess", "“好喔，我记住啦！”多多把写满歪歪扭扭数字的羊皮纸随意卷了卷塞进帽子里"},
	{"strPropCleared", "记了你的属性的羊皮纸我可丢掉了喔，想要留下来做个纪念么？"},
	{"strRuleErr", "规则数据获取失败,具体信息:\n"},
	{"strRulesFailedErr", "请求失败,无法连接数据库"},
	{"strPropDeleted", "多多伸出爪子，在羊皮纸上挖出一个小洞：“好，删掉了！”"},
	{"strPropNotFound", "“可是你根本没记录过这样的属性呀……”多多再次检查了一遍手里的羊皮纸"},
	{"strRuleNotFound", "服务器上好像没有记录这样的规则呢"},
	{"strProp", "“{0}的{1}我记得只有……{2}来着。”多多戳着手里的羊皮纸，“应该够用了吧？”"},
	{"strStErr", "你输错格式啦笨蛋两脚兽，试试这样：.st力量90|外貌5|智力5……"},
	{"strRulesFormatErr", "你输错格式啦笨蛋两脚兽，试试这样：.rules COC7:力量"},
	{"strJrrp", "{0}今天能捉到{1}条鱼！记得分我一半！"},
	{"strJrrpErr", "啊，好像现在捉不到鱼了…… 错误信息: \n{0}"},
	{"strFumble", "大失败\n“糟糕，大失败了——”多多慌张地挥舞着手里的骰子，“快把这个倒霉的家伙绑在锚上沉到海里去！”"},
	{"strFailure", "失败\n“喵，不怪我，”多多小声嘟囔着舔舔爪子，“这也是没有办法的事情嘛……”"},
	{"strSuccess", "成功\n“我很厉害嘛！”多多摇晃着尾巴。"},
	{"strHardSuccess", "困难成功\n“夸我夸我！”多多拨弄着骰子"},
	{"strExtremeSuccess", "极难成功\n“喔喔！”多多睁大眼睛，“好期待接下来发生的事情——”"},
	{"strSCFumble", "哎呀呀，是大失败..."},
	{"strCriticalSuccess", "大成功\n“喵？！！”多多蹦下瞭望台，叼着骰子跑到你面前，“怎！么！样！请开始你的表演——”"},
	{"strSCFailure", "“啊……失败了，”多多担心地甩甩尾巴，“还清醒吗？还没疯吧？”"},
	{"strSCSuccess", "成功了！"},
	{"strENFailure", "“失败了，好可惜！”多多凑过来看看你的人物卡"},
	{"strENSuccess", "“成功了！！”多多看起来比你还兴奋"},
	{"strInitListClearedNotice", "成功清除先攻记录!"},
	{"strInitListIsEmptyErr", "错误: 先攻列表为空!"},
	{"strCommandNotAvailableErr", "这个命令只能在群或者讨论组中使用喔"},
	{"strSuccessfullyEnabledNotice", "多多伸了个懒腰，从瞭望台上探出头来：“轮到我当船长了吗？”（成功启用）"},
	{"strAlreadyEnabledErr", "“别喊啦别喊啦，我有在好好工作诶！”多多不耐烦地晃晃尾巴（已处于启用状态）"},
	{"strSuccessfullyDisabledNotice", "“我去睡一会儿，就一小会儿……”多多嘟囔着窜上瞭望台（成功停用）"},
	{"strAlreadyDisabledErr", "多多打着呼噜翻了个身（已处于停用状态）"},
	{"strObCommandSuccessfullyEnabledNotice", "成功在这里启用旁观模式啦!"},
	{"strObCommandAlreadyEnabledErr", "错误: 这里的旁观模式已经被启用啦!"},
	{"strObCommandSuccessfullyDisabledNotice", "成功在这里禁用旁观模式啦!"},
	{"strObCommandAlreadyDisabledErr", "错误: 这里的旁观模式已经被禁用啦!"},
	{"strObCommandDisabledErr", "管理员不让在这里用旁观模式诶!"},
	{"strObListClearedNotice", "成功删除所有旁观者啦!"},
	{"strJrrpCommandSuccessfullyEnabledNotice", "“快去捉鱼，快去捉鱼！”多多小声催促着。（jrrp启用成功）"},
	{"strJrrpCommandAlreadyEnabledErr", "错误: 这里的.jrrp命令已经被启用!"},
	{"strJrrpCommandSuccessfullyDisabledNotice", "成功这里的禁用.jrrp命令!"},
	{"strJrrpCommandAlreadyDisabledErr", "错误: 这里的.jrrp命令已经被禁用!"},
	{"strJrrpCommandDisabledErr", "管理员已在这里禁用.jrrp命令!"},
	{"strHelpCommandSuccessfullyEnabledNotice", "成功这里的启用.help命令!"},
	{"strHelpCommandAlreadyEnabledErr", "错误: 这里的.help命令已经被启用!"},
	{"strHelpCommandSuccessfullyDisabledNotice", "成功这里的禁用.help命令!"},
	{"strHelpCommandAlreadyDisabledErr", "错误: 这里的.help命令已经被禁用!"},
	{"strHelpCommandDisabledErr", "管理员已在这里禁用.help命令!"},
	{"strRoomRuleSetErr","大成功大失败概率无效，请输入0-50内的整数喵！"},
	{"strRoomRuleClear","已将房规还原为默认值啦！（大成功1-5，大失败96-100）"},
	{"strRoomRuleClearErr","现在没有设置房规喔——"},
	{"strRoomRuleSet","房规设置成功了喵！"},
	{"strHelpMsg" , Dice_Short_Ver + "\n" + "请使用!dismiss 2749573030命令让多多自动退群喔！\n禁言和踢群会被拉黑，请务必注意——\n更多信息请扫描二维码——[CQ:image,file=HELP.jpg]"}
};