#include "StdAfx.h"
#include "HeartBeatThread.h"


CHeartBeatThread::CHeartBeatThread(void)
{
}


CHeartBeatThread::~CHeartBeatThread(void)
{
	CloseHandle(m_hTMWReporThreadHandle);
}


unsigned __stdcall CHeartBeatThread::ThreadFunction(LPVOID lpArg)
{
	DWORD dwTimeC = GetTickCount();
	ULONG lTime = 10000; // 15�� ���� ������� Socket.IO �� ����Ǿ� �־� 10�ʸ��� ����.

	CCaloris* pCaloris = (CCaloris*)lpArg;

	while(1)
	{	
		while(GetTickCount() - dwTimeC >= lTime)
		{
			OutputDebugStringA("Send Heart Beat Thread\n");
			if( -1 == pCaloris->SendHeartbeat())
			{
				return 0;
			}

			dwTimeC = GetTickCount();			
		}		
		Sleep(000);
	}
}


HANDLE CHeartBeatThread::SendHeartBeatThread(CCaloris* pCaloris)
 {	
	m_hTMWReporThreadHandle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, (LPVOID)pCaloris, 0, &m_dwTMWReporThreadId);
	return m_hTMWReporThreadHandle;	
}