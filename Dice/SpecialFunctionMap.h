#pragma once

#include <map>

#ifndef SpecialFunctionMap
#define SpecialFunctionMap

struct CocktailType
{
	CocktailType(std::string a="", std::string b="") : CocktailName(a), CocktailData(b)
	{
	}
	std::string CocktailName, CocktailData;
};
extern std::map<const int, CocktailType> CocktailList;
extern std::map<const int, std::string> SensojiTempleDivineSign;
extern std::map<const int, std::string> Characteristic;

#endif // !SpecialFunctionMap
