#include "StdAfx.h"
#include "FormatUtils.h"
#include "assert.h"
#include "atlconv.h"

string FormatUtils::vformat( LPCSTR fmt, va_list vl )
{
	CHAR buf[MAX_GSTRINGBUF + 1];
	int nBuf = _vsnprintf_s( buf, MAX_GSTRINGBUF, fmt, vl );
	
	assert( nBuf >= 0 );

	if( nBuf < 0 )
	{
		buf[MAX_GSTRINGBUF] = 0;
	}

	return string( buf );
}

wstring FormatUtils::vformat( LPCWSTR fmt, va_list vl )
{
	WCHAR buf[MAX_GSTRINGBUF + 1];
	int nBuf = _vsnwprintf_s( buf, MAX_GSTRINGBUF, fmt, vl );

	assert( nBuf >= 0 );

	if( nBuf < 0 )
	{
		buf[MAX_GSTRINGBUF] = 0;
	}

	return wstring( buf );
}

string FormatUtils::format( LPCSTR fmt, ... )
{
	va_list vl;
	va_start( vl, fmt );
	string ret = vformat( fmt, vl );
	va_end( vl );

	return ret;
}

wstring FormatUtils::format( LPCWSTR fmt, ... )
{
	va_list vl;
	va_start( vl, fmt );
	wstring ret = vformat( fmt, vl );
	va_end( vl );

	return ret;
}

int FormatUtils::AnsiToUTF16( const string& strAnsi, wstring& strUnicode )
{
	int len = ::MultiByteToWideChar( CP_ACP, 0, strAnsi.c_str(), -1, NULL, 0 );
	if( len == 0 )
	{
		return len;
	}
	
	wchar_t* wbuf = new wchar_t[len];
	len = ::MultiByteToWideChar( CP_ACP, 0, strAnsi.c_str(), -1, wbuf, len );
	if( len != 0 )
	{
		strUnicode = wbuf; 
	}

	delete[] wbuf;
	
	return len;
}

int FormatUtils::UTF16ToUTF8( const wstring& strUnicode, string& strUtf8 )
{
	int len = ::WideCharToMultiByte( CP_UTF8, 0, strUnicode.c_str(), -1, NULL, 0, NULL, NULL );
	if( len == 0 )
	{
		return len;
	}

	char* buf = new char[len];
	len = ::WideCharToMultiByte( CP_UTF8, 0, strUnicode.c_str(), -1, buf, len, NULL, NULL );
	if( len != 0 )
	{
		strUtf8 = buf;
	}

	delete[] buf;
	
	return len;
}

int FormatUtils::AnsiToUTF8( const string& strAnsi, string& strUtf8 )
{
	wstring strUnicode;
	if( AnsiToUTF16( strAnsi, strUnicode ) != 0 )
	{
		return UTF16ToUTF8( strUnicode, strUtf8 );
	}

	return 0;
}

int FormatUtils::UTF8ToUTF16( const string& strUtf8, wstring& strUnicode )
{   
	int len = ::MultiByteToWideChar( CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0 );
	if( len == 0 )
	{
		return len;
	}

	wchar_t* wbuf = new wchar_t[len];
	len = ::MultiByteToWideChar( CP_UTF8, 0, strUtf8.c_str(), -1, wbuf, len );
	if( len != 0 )
	{
		strUnicode = wbuf;
	}

	delete[] wbuf;
	
	return len;
}

int FormatUtils::UTF16ToAnsi( const wstring& strUnicode, string& strAnsi )
{
	int len = ::WideCharToMultiByte( CP_ACP, 0, strUnicode.c_str(), -1, NULL, 0, NULL, NULL );
	if( len == 0 )
	{
		return len;
	}

	char* buf = new char[len];
	len = ::WideCharToMultiByte( CP_ACP, 0, strUnicode.c_str(), -1, buf, len, NULL, NULL );
	if( len != 0 )
	{
		strAnsi = buf;
	}

	delete[] buf;
	
	return len;
}

int FormatUtils::UTF8ToAnsi( const string& strUtf8, string& strAnsi )
{
	wstring strUnicode;
	if( UTF8ToUTF16( strUtf8, strUnicode ) != 0 )
	{
		return UTF16ToAnsi(strUnicode, strAnsi);
	}

	return 0;
}

///////////////////////////////////////////////////////////
wstring FormatUtils::AnsiToUTF16( const string& strAnsi )
{
	wstring strUnicode = L"";

	AnsiToUTF16( strAnsi, strUnicode );

	return strUnicode;
}

wstring FormatUtils::UTF8ToUTF16( const string& strUtf8 )
{   
	wstring strUnicode = L"";

	UTF8ToUTF16( strUtf8, strUnicode );

	return strUnicode;
}

string FormatUtils::UTF16ToUTF8( const wstring& strUnicode )
{
	string strUtf8 = "";

	UTF16ToUTF8( strUnicode, strUtf8 );

	return strUtf8;
}

string FormatUtils::AnsiToUTF8( const string& strAnsi )
{
	string strUtf8 = "";

	AnsiToUTF8( strAnsi, strUtf8 );

	return strUtf8;
}

string FormatUtils::UTF16ToAnsi( const wstring& strUnicode )
{
	string strAnsi = "";

	UTF16ToAnsi( strUnicode, strAnsi );

	return strAnsi;
}

string FormatUtils::UTF8ToAnsi( const string& strUtf8 )
{
	string strAnsi = "";

	UTF8ToAnsi( strUtf8, strAnsi );

	return strAnsi;
}


/*
static FormatUtils::string WideToAnsi(const wstring& strUnicode)
{
	//WideCharToMultiByte(CP_ACP, 0, strUnicode.c_str(), len, str, len, NULL, NULL);
}


static FormatUtils::wstring AnsiTiWide(const string& strAnsi)
{

}
*/

string FormatUtils::replaceAll( const string &str, const string &pattern, const string &replace )  
{  
	string result = str;  
	string::size_type pos = 0;  
	string::size_type offset = 0;  

	while((pos = result.find(pattern, offset)) != string::npos)  
	{  
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);  
		offset = pos + replace.size();  
	}  

	return result;  
}  

vector<string> FormatUtils::split( string strSource, string strToken )
{
	int cutAt;
//	int index = 0;

	vector<string> vecResult;

	while( ( cutAt = strSource.find_first_of( strToken ) ) != strSource.npos )
	{
		if( cutAt > 0 )
		{
			vecResult.push_back( strSource.substr( 0, cutAt ) );
		}
		strSource = strSource.substr( cutAt + 1 );
	}

	if( strSource.length() > 0 )
	{
		vecResult.push_back( strSource.substr( 0, cutAt ) );
	}

	return vecResult;	
}

BOOL FormatUtils::IsNumeric( const string& s )
{
	BOOL decimal = FALSE;
	for( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		if( s[i] == '.' && !decimal )
		{
			decimal = TRUE; 
		}
		else if( s[i] < '0' || s[i] > '9' )
		{
			return FALSE;
		}
	}

	return TRUE;
}

// Add 2012.07

size_t FormatUtils::UnicodeToUTF8(wchar_t uc, char* UTF8)
{
	size_t tRequiredSize = 0;

	if (uc <= 0x7f)
	{
		if( NULL != UTF8 )
		{
			UTF8[0] = (char) uc;
			UTF8[1] = (char) '\0';
		}
		tRequiredSize = 1;
	}
	else if (uc <= 0x7ff)
	{
		if( NULL != UTF8 )
		{
			UTF8[0] = (char)(0xc0 + uc / (0x01 << 6));
			UTF8[1] = (char)(0x80 + uc % (0x01 << 6));
			UTF8[2] = (char) '\0';
		}
		tRequiredSize = 2;
	}
	else if (uc <= 0xffff)
	{
		if( NULL != UTF8 )
		{
			UTF8[0] = (char)(0xe0 + uc / (0x01 <<12));
			UTF8[1] = (char)(0x80 + uc / (0x01 << 6) % (0x01 << 6));
			UTF8[2] = (char)(0x80 + uc % (0x01 << 6));
			UTF8[3] = (char) '\0';
		}
		tRequiredSize = 3;
	}
	return tRequiredSize;
}


size_t FormatUtils::UTF8ToUnicode(char* UTF8, wchar_t& uc)
{
	size_t tRequiredSize = 0;
	uc = 0x0000;

	// ASCII byte 
	if( 0 == (UTF8[0] & 0x80) )
	{
		uc = UTF8[0];
		tRequiredSize = 1;
	}
	else // Start byte for 2byte
		if( 0xC0 == (UTF8[0] & 0xE0) && 0x80 == (UTF8[1] & 0xC0) )
		{
			uc += (UTF8[0] & 0x1F) << 6;
			uc += (UTF8[1] & 0x3F) << 0;
			tRequiredSize = 2;
		}
		else // Start byte for 3byte
			if( 0xE0 == (UTF8[0] & 0xE0) &&
				0x80 == (UTF8[1] & 0xC0) &&
				0x80 == (UTF8[2] & 0xC0) )
			{
				uc += (UTF8[0] & 0x1F) << 12;
				uc += (UTF8[1] & 0x3F) << 6;
				uc += (UTF8[2] & 0x3F) << 0;
				tRequiredSize = 3;
			}
			else
			{
				// Invalid case
				assert(false);
			}

			return tRequiredSize;
}

size_t FormatUtils::UnicodeStrToUTF8Str(wchar_t *szUni,char *szUTF8)
{
	size_t tRequiredSize = 0;

	int i=0;
	for(i=0;szUni[i];i++)
	{

		size_t tSize = 0;

		if( NULL != szUTF8 )
			tSize = UnicodeToUTF8(szUni[i], szUTF8 + tRequiredSize);
		else
			tSize = UnicodeToUTF8(szUni[i], NULL);
		tRequiredSize += tSize;
	}

	return tRequiredSize;
}



size_t FormatUtils::UTF8StrToUnicodeStr(char* szUTF8, size_t tUTF8Len, wchar_t* szUni)
{
	size_t tReadPos = 0;

	size_t i=0;
	for(i=0; tReadPos<tUTF8Len; i++)
	{
		wchar_t tTempUnicodeChar = 0;
		size_t tSize = UTF8ToUnicode(szUTF8 + tReadPos, tTempUnicodeChar);

		if( NULL != szUni )
			szUni[i] = tTempUnicodeChar;
		tReadPos += tSize;
	}

	return i;
}
