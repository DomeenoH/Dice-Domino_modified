#include <tchar.h>
#include <Windows.h>
#include "Constchar_to_LPCWSTR.h"

LPCWSTR to_LPCWSTR(const char *Buf)
{
	int LenBuf = MultiByteToWideChar(0, 0, Buf, -1, NULL, 0);
	wchar_t *LpwBuf = new wchar_t[LenBuf];
	MultiByteToWideChar(0, 0, Buf, -1, LpwBuf, LenBuf);
	return LpwBuf;
}