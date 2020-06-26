#include <iostream>
#include <cstring> // for strcat()
#include <io.h>
#include <string>
#include <map>
#include <fstream>
#include "MemoRecorder.h"
#include "TemClean.h"

using namespace std;


/*返回MEMO存储的图片的名称，包含.cqimg，map的second暂时没用*/
map<string, long long>FindMemoImg()
{
	map<long long, MemoType>::iterator MemoCount = MemoMap.begin();
	map<string, long long>ReturnMemoImg;
	string MemoGet, ImgName;
	while (MemoCount != MemoMap.end())
	{
		MemoGet = MemoCount->second.Display();
		while (MemoGet.find("[CQ:image,file=") != string::npos)
		{
			if (MemoGet.find(".jpg]") != string::npos)
			{
				ImgName = MemoGet.substr(MemoGet.find("[CQ:image,file=") + 15, (MemoGet.find(".jpg]") + 4) - (MemoGet.find("[CQ:image,file=") + 15)) + ".cqimg";
				MemoGet.erase(MemoGet.find(".jpg]", 4));
			}
			else if (MemoGet.find(".gif]") != string::npos)
			{
				ImgName = MemoGet.substr(MemoGet.find("[CQ:image,file=") + 15, (MemoGet.find(".gif]") + 4) - (MemoGet.find("[CQ:image,file=") + 15)) + ".cqimg";
				MemoGet.erase(MemoGet.find(".gif]", 4));
			}
			else if (MemoGet.find(".png]") != string::npos)
			{
				ImgName = MemoGet.substr(MemoGet.find("[CQ:image,file=") + 15, (MemoGet.find(".png]") + 4) - (MemoGet.find("[CQ:image,file=") + 15)) + ".cqimg";
				MemoGet.erase(MemoGet.find(".png]", 4));
			}
			else
				continue;
			ReturnMemoImg[ImgName] = 1;
			MemoGet.erase(MemoGet.find("[CQ:image,file="), 15);
		}
		MemoCount++;
	}
	return ReturnMemoImg;
}

/*清除除了memo里记录的图片以外的cqimg*/
void TemCQImgClean()
{
	char CQTemImg[200] = ".\\data\\image\\*.cqimg";
	map<string, long long>MemoImg = FindMemoImg();
	intptr_t handle;
	_finddata_t findData;
	handle = _findfirst(CQTemImg, &findData);
	do
	{
		string ImgName = findData.name;
		if (!MemoImg.count(ImgName))
		{
			string strDelPos = ".\\data\\image\\" + ImgName;
			const char *cstDelPos = strDelPos.c_str();
			remove(cstDelPos);
		}
	} while (!_findnext(handle,&findData));
	_findclose(handle);
}