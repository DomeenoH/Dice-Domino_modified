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
	{"strNameNumTooBig", "�棬���һʱ�����벻����ô������ѽ!������10��!"},
	{"strNameNumCannotBeZero", "���������㵽��Ҫ��Ҫ�������֣������������"},
	{"strSetInvalid", "�������û����ô��ֵ�����!������1-99999֮�������!"},
	{"strSetTooBig", "��ô���桭������һ�����!������1-99999֮�������!"},
	{"strSetCannotBeZero", "0�����Ǹ�ʲô��ֵ�������!������1-99999֮�������!"},
	{"strCharacterCannotBeZero", "���������㵽��Ҫ��Ҫ�Ұ�æ�������ԣ������������"},
	{"strCharacterTooBig", "��̫����̫����!�������æ���ҵؼ�¼�����ӳ�Ŀ�������в��У����һ��10���������ٶ��ˣ�����"},
	{"strCharacterInvalid", "���û�㶮���ָ�����.coc3��"},
	{"strSCInvalid", "���������scָ������������������������ͣ�����ʽΪ�ɹ���San/ʧ�ܿ�San,���û�м�¼sanֵ�Ļ�������ҲҪ���ϡ�����1/1d6 50���ԣ���"},
	{"strSanInvalid", "���sanֵ�������,���������ֵ��Χ��1-99�ڵ������!"},
	{"strEnValInvalid", "��ļ���ֵ�������,����1-99��Χ�ڵ��������ԣ�"},
	{"strNameDelErr", "���������ޣ��������û�и�����������ְɣ�"},
	{"strValueErr", "��࿴�����ָ�����һС�����������û������⵽����ʲô��˼"},
	{"strInputErr", "��࿴�����ָ�����һС�����������û������⵽����ʲô��˼"},
	{"strUnknownErr", "��������������ˡ������ǵ��������ﰡ���������߬��һ�����Ӳ�֪����"},
	{"strMEDisabledErr", "��ร�����.me����ص�������"},
	{"strGroupIDInvalid", "��෭�˰��죬û���ҵ�������Ⱥ"},
	{"strSendErr", "��Ϣ����ʧ���ˣ�Ϊʲô����������"},
	{"strDisabledErr", "�����ǣ������Ǳ߲�����˵��ѽ������(�����޷�ִ��:������ڴ�Ⱥ�б��ر�!)"},
	{"strUnableToGetErrorMsg", "�޷���ȡ������Ϣ!"},
	{"strDiceTooBigErr", "����һ�£��ȡ��������������æ���ҵر���һ������ӣ����ÿ�Ҫ�޳���������ô�������Ҹ������������ۣ���"},
	{"strRequestRetCodeErr", "���ʷ�����ʱ��������ˣ���������Ļ������԰ٶ�һ��HTTP״̬��: {0}"},
	{"strRequestNoResponse", "������δ�����κ���Ϣ"},
	{"strTypeTooBigErr", "�⡭��������Բ�����ĸ�������������Ŀ����"},
	{"strZeroTypeErr", "��������Ӻ���֡�������࿴�����ﲻ����״�����������㱡�"},
	{"strAddDiceValErr", "������Ҫ�����������ӵ�ʲôʱ���(��������ȷ�ļ�������:5-10֮�ڵ�����)"},
	{"strZeroDiceErr", "���ҵ����ӵ�����ȥ�ˣ����������𡪡��������������ֻ¶��ҡ���ŵ�β��"},
	{"strRollTimeExceeded", "̫����̫���ˣ���"},
	{"strRollTimeErr", "�쳣����������"},
	{"strWelcomeMsgClearedNotice", "��Ժ����·�²�ϴ������Ǿ��Լ�����ɣ�"},
	{"strWelcomeMsgIsEmptyErr", "�������û������ȥ�����ϴ��ķ�²�"},
	{"strWelcomeMsgUpdatedNotice", "֪�������Ժ����µķ�²��׽�ϴ��ˣ��Ҿ���ô���������ٺ١���"},
	{"strPermissionDeniedErr", "��ʲôʱ��������ҵĴ󸱣����ߴ���ҳ�Ϊ�µĴ������������ҷ���ʩ��ɣ�"},
	{"strNameTooLongErr", "�������Ĺ������֡������ó����Ҽǲ�ס��"},
	{"strUnknownPropErr", "���и��߹�����������𡭡������ܱ��������˹�����"},
	{"strEmptyWWDiceErr", "��ʽ����:��ȷ��ʽΪ.w(w)XaY!����X��1, 5��Y��10"},
	{"strPropErr", "�������������Բ�����������ģ����ټ��һ���ǲ�������д���ˡ���"},
	{"strSetPropSuccess", "����ร��Ҽ�ס����������д������ŤŤ���ֵ���Ƥֽ������˾�����ñ����"},
	{"strPropCleared", "����������Ե���Ƥֽ�ҿɶ�����ร���Ҫ��������������ô��"},
	{"strRuleErr", "�������ݻ�ȡʧ��,������Ϣ:\n"},
	{"strRulesFailedErr", "����ʧ��,�޷��������ݿ�"},
	{"strPropDeleted", "������צ�ӣ�����Ƥֽ���ڳ�һ��С�������ã�ɾ���ˣ���"},
	{"strPropNotFound", "�����������û��¼������������ѽ����������ٴμ����һ���������Ƥֽ"},
	{"strRuleNotFound", "�������Ϻ���û�м�¼�����Ĺ�����"},
	{"strProp", "��{0}��{1}�Ҽǵ�ֻ�С���{2}���š����������������Ƥֽ����Ӧ�ù����˰ɣ���"},
	{"strStErr", "������ʽ�����������ޣ�����������.st����90|��ò5|����5����"},
	{"strRulesFormatErr", "������ʽ�����������ޣ�����������.rules COC7:����"},
	{"strJrrp", "{0}������׽��{1}���㣡�ǵ÷���һ�룡"},
	{"strJrrpErr", "������������׽�������ˡ��� ������Ϣ: \n{0}"},
	{"strFumble", "��ʧ��\n����⣬��ʧ���ˡ����������ŵػ�������������ӣ�����������ù�ļһ����ê�ϳ�������ȥ����"},
	{"strFailure", "ʧ��\n�����������ң������С�����������צ�ӣ�����Ҳ��û�а취�����������"},
	{"strSuccess", "�ɹ�\n���Һ�����������ҡ����β�͡�"},
	{"strHardSuccess", "���ѳɹ�\n�����ҿ��ң�����ದŪ������"},
	{"strExtremeSuccess", "���ѳɹ�\n���ร�����������۾��������ڴ����������������顪����"},
	{"strSCFumble", "��ѽѽ���Ǵ�ʧ��..."},
	{"strCriticalSuccess", "��ɹ�\n�����������������²t��̨�����������ܵ�����ǰ��������ô�������뿪ʼ��ı��ݡ�����"},
	{"strSCFailure", "��������ʧ���ˣ�����ൣ�ĵ�˦˦β�ͣ����������𣿻�û��ɣ���"},
	{"strSCSuccess", "�ɹ��ˣ�"},
	{"strENFailure", "��ʧ���ˣ��ÿ�ϧ�������չ�������������￨"},
	{"strENSuccess", "���ɹ��ˣ�������࿴�������㻹�˷�"},
	{"strInitListClearedNotice", "�ɹ�����ȹ���¼!"},
	{"strInitListIsEmptyErr", "����: �ȹ��б�Ϊ��!"},
	{"strCommandNotAvailableErr", "�������ֻ����Ⱥ������������ʹ���"},
	{"strSuccessfullyEnabledNotice", "������˸��������Ӳt��̨��̽��ͷ�������ֵ��ҵ��������𣿡����ɹ����ã�"},
	{"strAlreadyEnabledErr", "�����������������ںúù�����������಻�ͷ��ػλ�β�ͣ��Ѵ�������״̬��"},
	{"strSuccessfullyDisabledNotice", "����ȥ˯һ�������һС����������������Ŵ��ϲt��̨���ɹ�ͣ�ã�"},
	{"strAlreadyDisabledErr", "�����ź��෭�˸����Ѵ���ͣ��״̬��"},
	{"strObCommandSuccessfullyEnabledNotice", "�ɹ������������Թ�ģʽ��!"},
	{"strObCommandAlreadyEnabledErr", "����: ������Թ�ģʽ�Ѿ���������!"},
	{"strObCommandSuccessfullyDisabledNotice", "�ɹ�����������Թ�ģʽ��!"},
	{"strObCommandAlreadyDisabledErr", "����: ������Թ�ģʽ�Ѿ���������!"},
	{"strObCommandDisabledErr", "����Ա�������������Թ�ģʽ��!"},
	{"strObListClearedNotice", "�ɹ�ɾ�������Թ�����!"},
	{"strJrrpCommandSuccessfullyEnabledNotice", "����ȥ׽�㣬��ȥ׽�㣡�����С���ߴ��š���jrrp���óɹ���"},
	{"strJrrpCommandAlreadyEnabledErr", "����: �����.jrrp�����Ѿ�������!"},
	{"strJrrpCommandSuccessfullyDisabledNotice", "�ɹ�����Ľ���.jrrp����!"},
	{"strJrrpCommandAlreadyDisabledErr", "����: �����.jrrp�����Ѿ�������!"},
	{"strJrrpCommandDisabledErr", "����Ա�����������.jrrp����!"},
	{"strHelpCommandSuccessfullyEnabledNotice", "�ɹ����������.help����!"},
	{"strHelpCommandAlreadyEnabledErr", "����: �����.help�����Ѿ�������!"},
	{"strHelpCommandSuccessfullyDisabledNotice", "�ɹ�����Ľ���.help����!"},
	{"strHelpCommandAlreadyDisabledErr", "����: �����.help�����Ѿ�������!"},
	{"strHelpCommandDisabledErr", "����Ա�����������.help����!"},
	{"strRoomRuleSetErr","��ɹ���ʧ�ܸ�����Ч��������0-50�ڵ���������"},
	{"strRoomRuleClear","�ѽ����滹ԭΪĬ��ֵ��������ɹ�1-5����ʧ��96-100��"},
	{"strRoomRuleClearErr","����û�����÷���ม���"},
	{"strRoomRuleSet","�������óɹ�������"},
	{"strHelpMsg" , Dice_Short_Ver + "\n" + "��ʹ��!dismiss 2749573030�����ö���Զ���Ⱥร�\n���Ժ���Ⱥ�ᱻ���ڣ������ע�⡪��\n������Ϣ��ɨ���ά�롪��[CQ:image,file=HELP.jpg]"}
};