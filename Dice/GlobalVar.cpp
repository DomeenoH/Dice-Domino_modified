/*
 *  _______     ________    ________    ________    __
 * |   __  \   |__    __|  |   _____|  |   _____|  |  |
 * |  |  |  |     |  |     |  |        |  |_____   |  |
 * |  |  |  |     |  |     |  |        |   _____|  |__|
 * |  |__|  |   __|  |__   |  |_____   |  |_____    __
 * |_______/   |________|  |________|  |________|  |__|
 *
 * Dice! QQ Dice Robot for TRPG
 * Copyright (C) 2018-2019 w4123���
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
 * �汾��Ϣ
 * �����޸�Dice_Build, Dice_Ver_Without_Build��DiceRequestHeader�Լ�Dice_Ver����
 * ���޸�Dice_Short_Ver��Dice_Full_Ver�����Դﵽ�汾�Զ���
 */
const unsigned short Dice_Build = 524;
const std::string Dice_Ver_Without_Build = "2.3.8";
const std::string DiceRequestHeader = "Dice/" + Dice_Ver_Without_Build;
const std::string Dice_Ver = Dice_Ver_Without_Build + "(" + std::to_string(Dice_Build) + ")";
const std::string Dice_Short_Ver = "Dice! by ���_����ŵħ�� Ver. " + Dice_Ver;
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
	{"strNameNumTooBig", "��Ҫ����ү������ô�����ָ��\n������1-10֮������֣�"},
	{"strNameNumCannotBeZero", "����ү������ħ��ร�\n��        ��\n�㿴����������Ǳ���ү����ȡ�����֣�"},
	{"strSetInvalid", "������ʲôĬ�����ۣ���\n������1-99999֮�������໣�"},
	{"strSetTooBig", "��ô�����Ĭ��������\n��������ñ���ү�������"},
	{"strSetCannotBeZero", "�����⡭����\n682˼���ţ���ͼ�ҳ�һ��0������ӡ�"},
	{"strCharacterCannotBeZero", "����������\n682Ϊ��������һ�������ڵĶ�����"},
	{"strCharacterTooBig", "��Ҫ������ô���˸�ʲô�����������������"},
	{"strCharacterInvalid", "�������ɴ�����Ч໣�\n������1-10֮������֣�"},
	{"strSCInvalid", "�㵽�׻᲻��Sancheck�ۣ�\n��ʽΪ�ɹ���San/ʧ�ܿ�San,��1/1d6��"},
	{"strSanInvalid", "�����Sanֵ����ô��������\n������1-99��Χ�ڵ�������"},
	{"strEnValInvalid", "�����Ǹ�ʲô��������\n������1-99��Χ�ڵ�������"},
	{"strNameDelErr", "������û���������ְɣ�"},
	{"strValueErr", "���ⷢ��ɶѽ��������ү��������(��ֱ��׳)"},
	{"strInputErr", "������������ʽ�������!"},
	{"strUnknownErr", "���ⷢ��ɶѽ��������ү��������(��ֱ��׳)"},
	{"strMEDisabledErr", "��໣��Ұ�.me����ص�������"},
	{"strGroupIDInvalid", "682���˰��죬û���ҵ�������Ⱥ"},
	{"strSendErr", "��Ϣ����ʧ���ˣ�Ϊʲô����������"},
	{"strDisabledErr", "�����ǣ������Ǳ߲�����˵��ѽ������(�����޷�ִ��:�����ڴ�Ⱥ�б��ر�!)"},
	{"strUnableToGetErrorMsg", "�Ǹ���������үҲ��֪�������������"},
	{"strDiceTooBigErr", "ιι����һ�£�\n��ô�����ӱ���ү������������������"},
	{"strRequestRetCodeErr", "��������Ҫ��ѯ�Ķ����������Ե��ˣ�\n HTTP״̬��: {0}"},
	{"strRequestNoResponse", "���񡭡�������������ү�ˣ���ô���£�"},
	{"strTypeTooBigErr", "���£��㶪�˸�ɶ�������������ˣ���\n����ţ�����үȥ�����ܵ�����ȥ�ˡ���"},
	{"strZeroTypeErr", "682������ǰ������״������ֹͣ��˼����"},
	{"strAddDiceValErr", "��Ҫ�ټ������ˣ���\n����ү��������ô�����Ӱ�������"},
	{"strZeroDiceErr", "��?����ү�����ӵ��Ķ�ȥ��?"},
	{"strRollTimeExceeded", "����ү�ɾ����㣬����ô���ֻᱻ����Ա��ˢ�������ģ�"},
	{"strRollTimeErr", "����������Ҫ�������֣���"},
	{"strWelcomeMsgClearedNotice", "����ү�Ѿ������Ⱥ�Ļ�ӭ�ʸ���������"},
	{"strWelcomeMsgIsEmptyErr", "��û���߹����µ�D����Ա����֮��Ӧ����ô˵����"},
	{"strWelcomeMsgUpdatedNotice", "����ү֪������\n�Ժ����µ�D����Ա�����Ҿ���ô˵����"},
	{"strPermissionDeniedErr", "�٣����ֲ��ǹ���Ա������ү������ģ�"},
	{"strNameTooLongErr", "������̫���ˣ�����ү�ǲ�ס��"},
	{"strUnknownPropErr", "��û���߹���������԰�����"},
	{"strEmptyWWDiceErr", "�������ʽ��໣�\n��ȷ��ʽΪ.w(w)XaY������X��1, 5��Y��10"},
	{"strPropErr", "������ү��������������԰�����"},
	{"strSetPropSuccess", "����ү��ס�����������"},
	{"strPropCleared", "����ү������������Զ���������"},
	{"strRuleErr", "���������ʲô����\n������Ϣ:\n"},
	{"strRulesFailedErr", "��������������������������أ�����ү���ʲ��ϰ�����"},
	{"strPropDeleted", "����ү�����������������"},
	{"strPropNotFound", "������û�и��߹�����ү��������԰���"},
	{"strRuleNotFound", "��������ү�ڹ������ϲ鲻�����������"},
	{"strProp", "�ۣ�{0}�����{1}��ֻ��{2}������Ҫ��Ҫ����ү��������������ֽţ�"},
	{"strStErr", "��ĸ�ʽ�������������\n�Ҹ���������.st�����ʹ�÷����ɣ�"},
	{"strRulesFormatErr", "������ʽ��:��ȷ��ʽΪ.rules[��������:]������Ŀ ��.rules COC:����"},
	{"strJrrp", "{0}�������ſ�������{1}����ɹ�����"},
	{"strJrrpErr", "���������JRRP���������Ե��ˣ�\n������Ϣ: \n{0}"},
	{"strFumble", "���١���ʧ�ܣ���������Լ����ģ�������үû�й�ϵ��\n682��KP���������õػ���β��"},
	{"strFailure", "���٣�ʧ���ˣ�"},
	{"strSuccess", "�ɹ���໣�"},
	{"strHardSuccess", "�޺����ѳɹ�Ү��"},
	{"strExtremeSuccess", "���ǡ������ѳɹ�໣���"},
	{"strSCFumble", "໣��Ǵ�ʧ�ܡ���"},
	{"strCriticalSuccess", "���ǡ�����ɹ�����\n682͵͵�������ӣ���Ҫ������һ�Ρ�"},
	{"strEnFailure", "�ۣ��������ʱ������������غ������ˡ���"},
	{"strEnSuccess", "�޺𣬾�Ȼ�ɹ��ˣ�"},
	{"strSCFailure", "��ʧ����໣���"},
	{"strSCSuccess", "���ɹ��ˣ�����"},
	{"strInitListClearedNotice", "�ɹ�����ȹ���¼!"},
	{"strInitListIsEmptyErr", "����: �ȹ��б�Ϊ��!"},
	{"strCommandNotAvailableErr", "�������ֻ����Ⱥ������������ʹ���"},
	{"strSuccessfullyEnabledNotice", "��໡�������\n682����������̽������Դ����ɹ�������"},
	{"strAlreadyEnabledErr", "����ɶ������682����ػ���β��\n(�����˴��ڿ���״̬)"},
	{"strSuccessfullyDisabledNotice", "����ཹ�ཡ�����ʲô�Ӵ�Ķ�������������صײ����ɹ��رգ�"},
	{"strAlreadyDisabledErr", "����صײ�ð����һ������\n(�����˴��ڹر�״̬)"},
	{"strObCommandSuccessfullyEnabledNotice", "�ɹ��ڱ�Ⱥ/�������������Թ�ģʽ!"},
	{"strObCommandAlreadyEnabledErr", "����: �ڱ�Ⱥ/�������Թ�ģʽ�Ѿ�������!"},
	{"strObCommandSuccessfullyDisabledNotice", "�ɹ��ڱ�Ⱥ/�������н����Թ�ģʽ!"},
	{"strObCommandAlreadyDisabledErr", "����: �ڱ�Ⱥ/�������Թ�ģʽ�Ѿ�������!"},
	{"strObCommandDisabledErr", "����Ա���ڱ�Ⱥ/�������н����Թ�ģʽ!"},
	{"strObListClearedNotice", "�ɹ�ɾ�������Թ���!"},
	{"strJrrpCommandSuccessfullyEnabledNotice", "�ɹ��ڱ�Ⱥ/������������.jrrp����!"},
	{"strJrrpCommandAlreadyEnabledErr", "����: �ڱ�Ⱥ/��������.jrrp�����Ѿ�������!"},
	{"strJrrpCommandSuccessfullyDisabledNotice", "�ɹ��ڱ�Ⱥ/�������н���.jrrp����!"},
	{"strJrrpCommandAlreadyDisabledErr", "����: �ڱ�Ⱥ/��������.jrrp�����Ѿ�������!"},
	{"strJrrpCommandDisabledErr", "����Ա���ڴ�Ⱥ/�������н���.jrrp����!"},
	{"strHelpCommandSuccessfullyEnabledNotice", "�ɹ��ڱ�Ⱥ/������������.help����!"},
	{"strHelpCommandAlreadyEnabledErr", "����: �ڱ�Ⱥ/��������.help�����Ѿ�������!"},
	{"strHelpCommandSuccessfullyDisabledNotice", "�ɹ��ڱ�Ⱥ/�������н���.help����!"},
	{"strHelpCommandAlreadyDisabledErr", "����: �ڱ�Ⱥ/��������.help�����Ѿ�������!"},
	{"strHelpCommandDisabledErr", "����Ա���ڴ�Ⱥ/�������н���.help����!"},
	{"strRoomRuleSetErr","��ɹ���ʧ�ܸ�����Ч��������0-50�ڵ�����໣�"},
	{"strRoomRuleClear","�ѽ����滹ԭΪĬ��ֵ��໣�����ɹ�1-5����ʧ��96-100��"},
	{"strRoomRuleClearErr","����û�����÷���໡���"},
	{"strRoomRuleSet","�������óɹ���໣�"},
	{"strHelpMsg" , Dice_Short_Ver + "\n" + "��ʹ��!dismiss 2602617724������682�Զ���Ⱥร�\n���Ժ���Ⱥ�ᱻ���ڣ������ע�⡪��\n������Ϣ��ɨ���ά�롪��[CQ:image,file=HELP.jpg]"}
};