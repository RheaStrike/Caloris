#include "stdafx.h"
#include <atlconv.h>
#include "EventSock.h"
#include <process.h>

using namespace std;

int CEventSock::m_nCountInstance;

CEventSock::CEventSock()
{
	m_Socket	= INVALID_SOCKET;
	m_hEvent[0] = WSA_INVALID_EVENT;
	m_hEvent[1] = WSA_INVALID_EVENT;

	CEventSock::m_bConnect = FALSE;
	m_nCountInstance++;
}

CEventSock::~CEventSock()
{
	if(m_Socket != INVALID_SOCKET)
		CloseSocket();

	Cleanup();
	m_nCountInstance--;
}


void CEventSock::Err_display(TCHAR *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
	OutputDebugString(msg);
	LocalFree(lpMsgBuf);
}


// 전 프로세스당 한번 ///////////////////////////////////////
int CEventSock::Startup()
{
	WORD	wVersionRequested;
	WSADATA	wsData;
	int		nError;

	wVersionRequested = MAKEWORD(2,2);
	nError = WSAStartup(wVersionRequested, &wsData);
	if(nError != 0)
	{
		OutputDebugString(_T("Fail : CEventSock::Startup()"));
	}

	return nError;
}


int CEventSock::Cleanup()
{
	int nError;
	nError = WSACleanup();
	if(nError == SOCKET_ERROR)
	{
		//Err_display(_T("Cleanup()"));
		OutputDebugStringA("CleantUp() Error");
	}
	return nError;
}
////////////////////////////////////////////////////////////////


SOCKET CEventSock::InitializeSocket()
{
	if(m_Socket != INVALID_SOCKET)
		CloseSocket();

	m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	m_hEvent[0] = WSACreateEvent();
	if(m_hEvent[0] == WSA_INVALID_EVENT)
		return (SOCKET)(SOCKET_ERROR);

	long Events = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE;
	if(WSAEventSelect(m_Socket, m_hEvent[0], Events) == SOCKET_ERROR)
		return (SOCKET)(SOCKET_ERROR);

	WSAIoctl(m_Socket,  FIONBIO, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	// 이벤트를 처리할 스레드를 동작시킨다.
	StartThreadEventSelect();
	
	return m_Socket;
}

int CEventSock::CloseSocket()
{
	int result = 0;

	if(m_Socket != INVALID_SOCKET)
	{
		StopThreadEventSelect();

		result = closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;

		if(WSACloseEvent(m_hEvent[0]) == SOCKET_ERROR)
			result = WSAGetLastError();
	}

	return result;
}


int CEventSock::Binding(UINT uPort)
{
	ZeroMemory(&m_SockAddr, sizeof(m_SockAddr));

	m_SockAddr.sin_family		= AF_INET;
	m_SockAddr.sin_addr.s_addr	= ADDR_ANY;
	m_SockAddr.sin_port			= htons(uPort);

	if(bind(this->m_Socket, (sockaddr*)&m_SockAddr, sizeof(m_SockAddr)))
		return WSAGetLastError();

	if(listen(this->m_Socket, SOMAXCONN))
		return WSAGetLastError();

	return 0;
}


int CEventSock::Send(const char *pSendData, int nDataSize)
{
	if(this->m_Socket != INVALID_SOCKET)
		return send(this->m_Socket, pSendData, nDataSize, 0);
	else
		return INVALID_SOCKET;
}


int CEventSock::Receive(char *pDataBuf, int nBufSize)
{
	if(this->m_Socket != INVALID_SOCKET)													
		return recv(this->m_Socket, pDataBuf, nBufSize, 0);
	else
		return INVALID_SOCKET;
}


int CEventSock::Connect(string strAddress, UINT uPort)
{
	ZeroMemory(&m_SockAddr, sizeof(m_SockAddr));

	USES_CONVERSION_EX;

	if (strAddress == "")
	{
		WSASetLastError (WSAEINVAL);
		return FALSE;
	}

	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = inet_addr(strAddress.c_str());

	if (m_SockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(strAddress.c_str());
		if (lphost != NULL)
			m_SockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
			WSASetLastError(WSAEINVAL);
			return FALSE;
		}
	}

	m_SockAddr.sin_port = htons((u_short)uPort);

	return connect(this->m_Socket, (sockaddr *)&m_SockAddr, sizeof(m_SockAddr));
}


SOCKET CEventSock::GetSocket()
{
	return m_Socket;
}


WSAEVENT* CEventSock::GetEvenHandle()
{
	return m_hEvent;
}

BOOL GetIPfromDomain(string lpcDomain)
{
	return FALSE;
}


// 스레드 시작 중지 함수
HANDLE CEventSock::StartThreadEventSelect()
{
	m_hEvent[1] = WSACreateEvent();
	m_hEventSelectThreadHandle = (HANDLE)_beginthreadex(NULL, 0, EventSelectThread, (LPVOID)this, 0, &m_uEventSelectThreadId);

	return m_hEventSelectThreadHandle;
}


int CEventSock::StopThreadEventSelect()
{
	WSASetEvent(m_hEvent[1]);
	if(WAIT_OBJECT_0 == WaitForSingleObject(m_hEventSelectThreadHandle, INFINITE))
	{
		CloseHandle(m_hEventSelectThreadHandle);
	}

	CloseHandle(m_hEvent[1]);

	return 0;
}


unsigned __stdcall CEventSock::EventSelectThread(LPVOID lpArg)
{
	CEventSock* pConnector = (CEventSock*)lpArg;

	if(NULL == pConnector)
		return 0;

	return pConnector->EventSelectThreadFunc();
}

unsigned CEventSock::EventSelectThreadFunc()
{
	DWORD dwWaitStatus;
	WSANETWORKEVENTS NetworkEvents;

	for( ; ; )
	{
		dwWaitStatus = WSAWaitForMultipleEvents(2, m_hEvent,FALSE, WSA_INFINITE, FALSE);

		if(dwWaitStatus == WSA_WAIT_FAILED)
		{
			cout<< _T("Error : ") << WSAGetLastError() << endl;
		}
		else if(dwWaitStatus == WSA_WAIT_EVENT_0)
		{
			int Index = dwWaitStatus - WSA_WAIT_EVENT_0;
			WSAEnumNetworkEvents(m_Socket, m_hEvent[0], &NetworkEvents);

			if(NetworkEvents.lNetworkEvents & FD_READ)
			{
				OnReceive(NetworkEvents.iErrorCode[FD_READ_BIT]);
			}

			if(NetworkEvents.lNetworkEvents & FD_WRITE)
			{
				OnSend(NetworkEvents.iErrorCode[FD_WRITE_BIT]);
			}

			if(NetworkEvents.lNetworkEvents & FD_CLOSE)
			{
				OnClose(NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
			}

			if(NetworkEvents.lNetworkEvents & FD_ACCEPT)
			{
				OnAccept(NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
			}

			if(NetworkEvents.lNetworkEvents & FD_CONNECT)
			{
				OnConnect(NetworkEvents.iErrorCode[FD_CONNECT_BIT]);
			}

			if(NetworkEvents.lNetworkEvents & FD_OOB)
			{
				OnOutOfBandData(NetworkEvents.iErrorCode[FD_OOB_BIT]);
			}
		}
		else if(dwWaitStatus == WSA_WAIT_EVENT_0+1)
		{
			cout<< _T("CEventSock Thread Quit.") << endl;
			break;
		}
	}
	cout << _T("Thread :: exit") << endl;
	return 0;
}


void CEventSock::OnReceive(int nErrorCode)
{
	char buf[1024] = {0,};
	Receive(buf, 1024);
}


void CEventSock::OnSend(int nErrorCode)
{
	cout << _T("CEventSock::OnSend : ") << nErrorCode<<endl;
}


void CEventSock::OnOutOfBandData(int nErrorCode)
{
	cout << _T("CEventSock::OnOutOfBandData : ") << nErrorCode;
}


void CEventSock::OnAccept(int nErrorCode)
{
	cout << _T("CEventSock::OnAccept : ") << nErrorCode;
}


void CEventSock::OnConnect(int nErrorCode)
{	
	if(0 == nErrorCode)
	{
		m_bConnect = TRUE;
	}
	else if(_SERVER_NOT_FOUND == nErrorCode)
	{
		m_bConnect = FALSE;
	}
	else if(_SERVER_REFUSE == nErrorCode)
	{
		m_bConnect = FALSE;
	}
}


void CEventSock::OnClose(int nErrorCode)
{
	m_bConnect = FALSE;
}
