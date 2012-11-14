#include "StdAfx.h"
#include "Caloris.h"
#include "Common/CommonUtils.h"
#include "Common/FormatUtils.h"
#include "Util/Base64.h"


CCaloris::CCaloris(void)
{
	if (!InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x00000400)) 
		return;

	m_pHeartBeatThread = NULL;
	m_Status = _NONE;
	Startup();
	InitializeSocket();
}


CCaloris::CCaloris(HWND* phWnd)
{	
	if (!InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x00000400)) 
		return;

	m_pParentWnd = phWnd;
	m_pHeartBeatThread = NULL;
	m_Status = _NONE;
	Startup();
 	InitializeSocket();
}


CCaloris::~CCaloris(void)
{	
	// 크리티컬섹션이 삭제되기 전에 이벤트 스레드를 종료시켜보자.
	CloseSocket();

	DeleteCriticalSection(&m_CriticalSection);
	if(m_pHeartBeatThread)
	{
		delete m_pHeartBeatThread;
		m_pHeartBeatThread = NULL;
	}

	m_Status = _NONE;
}		


int CCaloris::ConnectToNodeJS(string strAddress, UINT uPort)
{
	int iRet = 0;
	if(!IsConnected())
		Connect(strAddress, uPort);
	
	return iRet;
}


void CCaloris::OnConnect(int nErrorCode)
{
	if(0 == nErrorCode)
	{		
		m_bConnect = TRUE;
		SendMessage(*m_pParentWnd, WM_PRINT_CON, (WPARAM)m_bConnect, 0);
		SendTimeStampAndGetSessionID();
	}
	else if(_SERVER_NOT_FOUND == nErrorCode)
	{
		m_bConnect = FALSE;
		SendMessage(*m_pParentWnd, WM_PRINT_CON, (WPARAM)m_bConnect, 0);
	}
	else if(_SERVER_REFUSE == nErrorCode)
	{
		m_bConnect = FALSE;
		SendMessage(*m_pParentWnd, WM_PRINT_CON, (WPARAM)m_bConnect, 0);
	}
}

int CCaloris::SendTimeStampAndGetSessionID()
{
	//
	// TimeStamp용 랜덤키 생성
	//
	m_Status = _SEND_TIMESTAMP;

	string strHead = "GET /socket.io/1/?t=%s&jsonp=0 HTTP/1.1\n\n";
	
	strHead = FormatUtils::format(strHead.c_str(), CommonUtils::GetRandomKey(13).c_str());

	return Send(strHead.c_str(), strHead.length());
}


int CCaloris::SendConnection()
{
	if(FALSE == m_bConnect)
		return 0;

	string payload = ("1::");
	payload = FormatUtils::AnsiToUTF8(payload);

	char WSFrame[131];
	WSFrame[0] = '\x81';
	WSFrame[1] = 128 + payload.length();
	WSFrame[2] = '\x00';
	WSFrame[3] = '\x00';
	WSFrame[4] = '\x00';
	WSFrame[5] = '\x00';
	memcpy(WSFrame + 6, payload.c_str(), payload.length());

	int iRet;
	if(this)
		iRet = Send(WSFrame, payload.length() + 6 );

	if(iRet <0)
		OutputDebugStringA("SendConnection() Error\n");
	return iRet;
}


int CCaloris::SendHeartbeat()
{
	if(FALSE == m_bConnect)
		return 0;

	string payload = ("2:::");
	payload = FormatUtils::AnsiToUTF8(payload);

	char WSFrame[131];
	WSFrame[0] = '\x81';
	WSFrame[1] = 128 + payload.length();
	WSFrame[2] = '\x00';
	WSFrame[3] = '\x00';
	WSFrame[4] = '\x00';
	WSFrame[5] = '\x00';
	memcpy(WSFrame + 6, payload.c_str(), payload.length());
   
	int iRet;
	if(this)
		iRet = Send(WSFrame, payload.length() + 6);

	return iRet;
}


int CCaloris::SendTestMessage(string strMessage)
{
	//string str = FormatUtils::UTF16ToAnsi(strMessage.c_str());
	string str = FormatUtils::AnsiToUTF8(strMessage.c_str());
	//Send(str.c_str(), str.length());

	packet test;
	test.m_Type = packet::event;
	test.m_data = str;

	CSocketIOPayloader payload;
	string sendData = payload.EncodePacket(test);
	Send(sendData.c_str(), sendData.length());

	return 1;
}


string CCaloris::GetSessionID()
{
	return m_SessionID;
}


void CCaloris::SetSessionID(string strSessionID)
{
	m_SessionID = strSessionID;
}


int CCaloris::SendSwitchProtocols(string& strSessionID)
{
	if(_RECV_SESSIONID != m_Status)
		return 0;

	string strSwitchingHead;
	strSwitchingHead = "Upgrade: websocket\n"
			   "Connection: Upgrade\n"
			   "Sec-WebSocket-Key: %s\n"  
			   "Sec-WebSocket-Version: 13\n"
			   "Sec-WebSocket-Extensions: x-webkit-deflate-frame\n\n"; 

	strSwitchingHead = FormatUtils::format(strSwitchingHead.c_str(), base64_encode(reinterpret_cast<unsigned char const*>(CommonUtils::GetRandomKey(16).c_str()), 16).c_str());
	string strHead = "GET /socket.io/1/websocket/" + strSessionID +  " HTTP/1.1\n" + strSwitchingHead.c_str();

	m_Status = _SEND_SWITCHING;

	return Send(strHead.c_str(), strHead.length());
}


void CCaloris::OnReceive(int nErrorCode)
{
	EnterCriticalSection(&m_CriticalSection); 

#ifdef _DEBUG
	OutputDebugString(_T("Recv Start\n"));
#endif // _DEBUG

	//
	// HTTP 헤더 받기와 일반 Socket.IO JSON 받기를 나누었다.
	//

	m_strRecv.clear();

	if(_SEND_TIMESTAMP == m_Status || _SEND_SWITCHING == m_Status)
	{
		vector<char> recvData;
		char buffer;
		int nReceive;

		while(nReceive = Receive(&buffer, sizeof(char)) > 0)
		{
			if(nReceive == SOCKET_ERROR)
			{
				int errorVal = WSAGetLastError();

				if(errorVal == WSAENOTCONN)
				{
					OutputDebugString(_T("Socket not connected!"));
				}
			}
			else
			{
				if(buffer == '\n')
				{
					for(int i=0; i < static_cast<int>(recvData.size()); i += 1)
					{
						m_strRecv += recvData[i];					
					}
					recvData.clear();
				}
				else
				{
					recvData.push_back(buffer);
				}
			}
		}
	}
	else if(_RECV_SWITCHING == m_Status)
	{
		static char szBuff[WS_FRAME_SIZE] = "\x0";
		int nReceive = Receive(szBuff, sizeof(szBuff));
		szBuff[nReceive] ='\x0';
		m_strRecv.assign(szBuff);

	}
	else if(_SOCKETIO_COMPLETE == m_Status)
	{
		if(m_parser.ProcessPacket(this, m_recvedMsg) == false)
		{
			return;
		}
	}

	int msgID = WM_UNDEFINED_MSG;
	switch(m_Status)
	{
	case _SEND_TIMESTAMP :
		{
			m_Status = _RECV_SESSIONID;

			size_t found1, found2;
			string key1("io.j[0](\"");
			string key2(":");
			found1 = m_strRecv.find(key1);
			
			if(string::npos != found1)
			{
				m_strRecv.replace(0, found1 + key1.length(), "");
				
				found2 = m_strRecv.find(key2);
				if(string::npos != found2)
				{
					m_strRecv.replace(found2, m_strRecv.length(), "");
					OutputDebugStringA(m_strRecv.c_str());
					SetSessionID(m_strRecv);
					
					SendSwitchProtocols(m_strRecv);										
					msgID = WM_PRINT_RECV;
				}
				else
				{
					// 서버에서 응답실패. 에러처리 필요
					msgID = WM_ERR_CONNECTION;
				}
			}
		}
		break;

	case _SEND_SWITCHING :
		{
			size_t found1, found2;
			string key1("HTTP/1.1 101 Switching Protocols");
			string key2("Sec-WebSocket-Accept: ");
			
			found1 = m_strRecv.find(key1);
			found2 = m_strRecv.find(key2);
		
			if(string::npos != found1 && string::npos != found2 )
			{
				//m_Status = _RECV_SWITCHING;			

				//SendConnection();

				m_Status = _SOCKETIO_COMPLETE;
				m_pHeartBeatThread = new CHeartBeatThread();
				m_pHeartBeatThread->SendHeartBeatThread(this);

				msgID = WM_COMPLETE_CONNECTION;
			}
			else
			{
				// 서버에서 응답실패. 에러처리 필요
				msgID = WM_ERR_CONNECTION;
			}
		}
		break;

	case _RECV_SWITCHING :
		{
			size_t found;
			string key("1::");

			found = m_strRecv.find(key);
			if(string::npos != found)
			{
				m_Status = _SOCKETIO_COMPLETE;
				m_pHeartBeatThread = new CHeartBeatThread();
				m_pHeartBeatThread->SendHeartBeatThread(this);
				
				return;
			}
		}
		break;

	case _SOCKETIO_COMPLETE :
		{
			// 스킵
			if(m_recvedMsg.m_Type == packet::heartbeat)
			{
				return;
			}

			Json::StyledWriter writer;
			Json::Reader reader;
			m_strRecv = writer.write(m_recvedMsg.m_data);			

			msgID = WM_RECV_MSG;
		}
		break;
	}
#ifdef _DEBUG
	OutputDebugStringA(m_strRecv.c_str());
	OutputDebugStringA("\n");
#endif // _DEBUG
	m_strRecv = FormatUtils::UTF8ToAnsi(m_strRecv);
#ifdef _DEBUG
	OutputDebugStringA(m_strRecv.c_str());
	OutputDebugStringA("\n");
#endif // _DEBUG
	//
	// Game에게 알려주는 방법
	//
	SendMessage(*m_pParentWnd, msgID, 0, 0);
	LeaveCriticalSection(&m_CriticalSection); 

#ifdef _DEBUG
	OutputDebugString(_T("Recv End\n"));
#endif // _DEBUG
}


void CCaloris::OnClose(int nErrorCode)
{
	m_bConnect = FALSE;
	m_SessionID = _NONE;
	
	if(m_pHeartBeatThread)
	{
		delete m_pHeartBeatThread;
		m_pHeartBeatThread = NULL;
	}

	SendMessage(*m_pParentWnd, WM_PRINT_CON, (WPARAM)m_bConnect, 0);
}


string CCaloris::GetRecvData()
{
	return m_strRecv;
}


int CCaloris::sendMessage(Json::Value& sendMsg)
{
	Json::StyledWriter writer;
	Json::Reader reader;

	string temp = writer.write(sendMsg);
	temp = FormatUtils::AnsiToUTF8(temp);
	if( false == reader.parse(temp, sendMsg))
	{
		return -1;
	}

	string sendData = m_parser.EncodePacket(sendMsg, packet::event);
	return Send(sendData.c_str(), sendData.length());
}