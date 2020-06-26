#include <string>
#include <algorithm>
#include "CDs_UpertoLower.h"

std::string tolower_inclideChinese(std::string s)
{
	std::string outPut;
	for (int i = 0; i != s.length(); i++)
	{
		if (s[i] > 0x80 || s[i] < 0)
		{
			char out[3] = { s[i], s[i + 1],'\0' };
			outPut += out;
			i++;
		}
		else
			outPut += tolower(s[i]);
	}
	return outPut;
}