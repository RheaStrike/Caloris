#include "stdafx.h"
#include "CommonUtils.h"
#include <time.h>

using namespace std;

string CommonUtils::CreateGuid()
{
	GUID Guid = {0};
	::CoCreateGuid( &Guid );

	WCHAR buf[GUID_LENGTH];
	CHAR* szGuid;

	StringFromGUID2(Guid, buf, GUID_LENGTH);
	szGuid = WCharToChar(buf);

	string strTempGuid;
	strTempGuid.assign(szGuid);
	strTempGuid.replace(0, 1, (""));
	strTempGuid.replace(36, 1,(""));

	return strTempGuid;
}


CHAR* CommonUtils::WCharToChar(const wchar_t* pwstrSrc)
{
#if !defined _DEBUG
	int len = 0;
	len = (wcslen(pwstrSrc) + 1)*2;
	char* pstr      = (char*) malloc ( sizeof( char) * len);
	WideCharToMultiByte( 949, 0, pwstrSrc, -1, pstr, len, NULL, NULL);
#else
	int nLen = wcslen(pwstrSrc);

	char* pstr = (char*)malloc(sizeof( char) * nLen + 1);
	size_t numberOfConverted = 0;
	wcstombs_s(&numberOfConverted, pstr, sizeof(char) * nLen + 1, pwstrSrc, sizeof(char) * nLen + 1);
#endif

	return pstr;
}


tstring CommonUtils::GetNowTime()
{
	tstring strTime;
	SYSTEMTIME st;
	TCHAR str[128] = { 0,};

	GetLocalTime(&st);
	wsprintf(str, _T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strTime.assign(str);

	return strTime;
}


vector<string> CommonUtils::GetLocalIPAddr()
{
	WSADATA wsa;
	IN_ADDR addr;

	char LocalName[256], IPAddr[15], i = 0;

	vector<string> vecIP;		

	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return vecIP;
	if(gethostname(LocalName, 256) == SOCKET_ERROR) return vecIP;

	HOSTENT *ptr = gethostbyname(LocalName);
	if(ptr == NULL) return vecIP;

	while(ptr->h_addr_list[i] != NULL)
	{
		memcpy(&addr, ptr->h_addr_list[i], ptr->h_length);
		sprintf_s(IPAddr, "%s", inet_ntoa(addr));

		vecIP.push_back(IPAddr);
		i++;
	}


	WSACleanup();
	return vecIP;
}


string CommonUtils::GetRandomKey(int iSize)
{
	srand(static_cast<int>(time(NULL)));
	UINT iRand;
	static char chRand[2];	
	ZeroMemory(chRand, sizeof(chRand));
	vector<char> vtRand;
	string strKey;

	for(int i=0; i < iSize; i++)
	{
		iRand = rand() % 10;
		_itoa_s(iRand, chRand, 10);
		strKey += chRand;
	}

	return strKey;	
}
