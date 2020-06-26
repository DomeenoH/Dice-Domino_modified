#pragma once
#include <tchar.h>
#include <Windows.h>

#ifndef Constchar_to_LPCWSTR
#define Constchar_to_LPCWSTR

LPCWSTR to_LPCWSTR(const char *Buf);

#endif //Constchar_to_LPCWSTR