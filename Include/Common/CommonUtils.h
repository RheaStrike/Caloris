#pragma once

#include <tchar.h>
#include <ObjBase.h>
#include <vector>
#include <string>
using namespace std;

// Unicode
typedef std::basic_string<TCHAR> tstring;

class CommonUtils
{
public:
	const static int GUID_LENGTH = 39;

	static CHAR*  WCharToChar(const wchar_t* pwstrSrc);
	static string CreateGuid();

	static tstring GetNowTime();
	static vector<string> GetLocalIPAddr();
	static string GetRandomKey(int iSize);
};

