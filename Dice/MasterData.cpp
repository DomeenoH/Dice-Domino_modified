#include <map>
#include"MasterData.h"

using namespace std;

long long MasterQQID = 287887313;/*请填入主人QQ号，用于发送反馈消息以及判断权限*/
map<long long, int> RoomRule;/*房规存储*/
map<long long, int> GroupTestCount;