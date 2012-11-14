#pragma once

#include "boost/thread.hpp"
#include "Caloris.h"

class CCaloris;

class CHeartBeatThread
{
public:
	CHeartBeatThread(void);
	virtual ~CHeartBeatThread(void);
	
	HANDLE SendHeartBeatThread(CCaloris* pCaloris);
	
private:
	static unsigned __stdcall ThreadFunction(LPVOID lpArg);
	HANDLE	m_hTMWReporThreadHandle;
	unsigned 	m_dwTMWReporThreadId;
};


