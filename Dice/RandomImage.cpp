#include <Windows.h>
#include <tchar.h>/*Ëæ»úÃ¨*/
#include <iostream>/*Ëæ»úÃ¨*/
#include <urlmon.h>/*Ëæ»úÃ¨*/
#include <wininet.h>/*Ëæ»úÃ¨*/
#pragma comment(lib,"WinInet.lib")/*Ëæ»úÃ¨*/
#pragma comment(lib, "urlmon.lib")/*Ëæ»úÃ¨*/
#include "RandomGenerator.h"
#include "Constchar_to_LPCWSTR.h"
#include "CQAPI_EX.h"
#include "RandomImage.h"

DWORD WINAPI CatImage(LPVOID lpParamter)
{
	char BackUpUrl[60] = "https://d2ph5fj80uercy.cloudfront.net/0";
	const int intrandomcata = RandomGenerator::Randint(1, 3);
	const int intrandomcatb = RandomGenerator::Randint(1, 8000);
	sprintf_s(BackUpUrl, "%s%d/cat%d.jpg", BackUpUrl, intrandomcata, intrandomcatb);
	LPCWSTR LpwUrl = to_LPCWSTR(BackUpUrl), File = _T("C:\\Users\\Administrator\\Desktop\\qingmulian\\data\\image\\cat.jpg");
	URLDownloadToFile(0, LpwUrl, File, 0, 0);
	return 1;
}

DWORD WINAPI WaifuImage(LPVOID lpParamter)
{
	char Url[60] = "https://www.thiswaifudoesnotexist.net/example-";
	const int intrandomwaifu = RandomGenerator::Randint(1, 99999);
	sprintf_s(Url, "%s%d.jpg", Url, intrandomwaifu);
	LPCTSTR LpwUrl = to_LPCWSTR(Url), File = _T("C:\\Users\\Administrator\\Desktop\\qingmulian\\data\\image\\waifu.jpg");
	URLDownloadToFile(0, LpwUrl, File, 0, 0);
	return 1;
}

DWORD WINAPI FoxImage(LPVOID lpParamter)
{
	HRESULT hr;
	LPCTSTR Url = _T("https://foxrudor.de/"), File = _T("C:\\Users\\Administrator\\Desktop\\qingmulian\\data\\image\\fox.jpg");
	DeleteUrlCacheEntry(Url);
	URLDownloadToFile(0, Url, File, 0, 0);
	return 1;
}

void exeFaceImage()
{
	HRESULT hr;
	LPCTSTR Url = _T("https://thispersondoesnotexist.com/image"), File = _T("C:\\Users\\Administrator\\Desktop\\qingmulian\\data\\image\\face.jpg");
	DeleteUrlCacheEntry(Url);
	URLDownloadToFile(0, Url, File, 0, 0);
	return;
}

DWORD WINAPI FaceImage(LPVOID lpParamter)
{
	exeFaceImage();
	return 1;
}