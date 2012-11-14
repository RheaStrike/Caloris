#pragma once

#define _SERVER_NOT_FOUND 10049
#define _SERVER_REFUSE	  10061

#include <iostream>
#include <string>

using namespace std;

class CEventSock  
{
public:	 
	CEventSock();
	virtual ~CEventSock();

	void Err_display(TCHAR *msg);

	static int	Startup();
	static int	Cleanup();
	SOCKET InitializeSocket();
	int	CloseSocket();

	int Connect(string lpcDomain, UINT uPort);
	int	Receive(char* pDataBuf, int nBufSize);
	int	Send(const char* pSendData, int nDataSize);
	int	Binding(UINT uPort);

	WSAEVENT*	GetEvenHandle();
	SOCKET		GetSocket();
	CEventSock* GetClientSocket() { return this;}
	BOOL		GetIPfromDomain(string lpcDomain);

	HANDLE StartThreadEventSelect();
	int StopThreadEventSelect();
		  
	BOOL IsConnected() { return m_bConnect; }

private:
	static unsigned __stdcall EventSelectThread(LPVOID lpArg);	
	static int m_nCountInstance;

	unsigned EventSelectThreadFunc();
	SOCKET m_Socket;
	sockaddr_in m_SockAddr;

	// Socket event handle: 0=Socket event, 1= End of thread
	WSAEVENT m_hEvent[2];
	HANDLE	m_hEventSelectThreadHandle;
	unsigned m_uEventSelectThreadId;

protected:
	volatile BOOL m_bConnect;
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnOutOfBandData(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};