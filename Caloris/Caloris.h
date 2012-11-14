#pragma once

#include "SocketUtil/EventSock.h"
#include "SocketIOPayloader.h"
#include "HeartBeatThread.h"
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

const int WM_UNDEFINED_MSG	= WM_USER + 100;
const int WM_PRINT_RECV		= WM_USER + 101;
const int WM_PRINT_CON		= WM_USER + 102;
const int WM_RECV_MSG		= WM_USER + 103;
const int WM_COMPLETE_CONNECTION = WM_USER + 104;
const int WM_ERR_CONNECTION = WM_USER + 105;

enum WS_STATUS
{
	_NONE = 0,
	_SEND_TIMESTAMP = 1,
	_RECV_SESSIONID = 2,
	_SEND_SWITCHING = 3,
	_RECV_SWITCHING = 4,
	_SOCKETIO_COMPLETE = 5,
};

static const int WS_FRAME_SIZE = 137;  // 131 + 6

class CHeartBeatThread;

class CCaloris : public CEventSock
{
public:
	CCaloris(void);
	CCaloris(HWND* phWnd);
	virtual ~CCaloris(void);
		
	int ConnectToNodeJS(string strAddress, UINT uPort);
	void OnConnect(int nErrorCode);
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);
	string GetSessionID();
	string GetRecvData();

	int SendTestMessage(string strMessage);
	void SetHWnd(HWND* phWnd) { m_pParentWnd = phWnd; }	
	int SendHeartbeat();
	int SendConnection();				   
	
	int sendMessage(Json::Value& sendMsg);
	//int sendMessage(packet& sendMsg);

private:
	int SendTimeStampAndGetSessionID();
	int SendSwitchProtocols(string& strSessionID);

	void SetSessionID(string strSessionID);
	CRITICAL_SECTION m_CriticalSection;

	string m_SessionID;
	WS_STATUS m_Status;
	CHeartBeatThread* m_pHeartBeatThread;
	string m_strRecv;
	HWND* m_pParentWnd;

	CSocketIOPayloader	m_parser;
	packet	m_recvedMsg;
};

