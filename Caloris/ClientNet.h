#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/regex.hpp> 

#include "chat_message.hpp"

#include "Msg.h"

using boost::asio::ip::tcp;

//typedef std::deque<CMsg> chat_message_queue;
typedef std::deque<chat_message> chat_message_queue;

class CClientNet
{
public:
	CClientNet(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: m_ioService(io_service),
		m_socket(io_service)
	{
		boost::asio::async_connect(m_socket, endpoint_iterator,
			boost::bind(&CClientNet::handle_connect, this,
			boost::asio::placeholders::error));
	}

	void write(const chat_message& msg)
	{
		m_ioService.post(boost::bind(&CClientNet::do_write, this, msg));
	}

	void close()
	{
		m_ioService.post(boost::bind(&CClientNet::do_close, this));
	}

private:
	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			// 타임 스탬프
			time_t curTime = time(NULL);
			LONGLONG curSec = curTime;

			char head[512]={0,};
			_snprintf(head, 512, "GET /socket.io/1/?t=%I64d&jsonp=0 HTTP/1.1\n\n\n",
				curSec);

			boost::asio::async_write(m_socket,
				boost::asio::buffer(head, strlen(head)),
				boost::bind(&CClientNet::handle_waite_handshake, this,
					boost::asio::placeholders::error));
		}
	}

	void handle_waite_handshake(const boost::system::error_code& error)
	{
		boost::asio::async_read(m_socket, m_revecMsg,
			boost::asio::transfer_at_least(1),
			boost::bind(&CClientNet::handle_handshakeing, this,
			boost::asio::placeholders::error));
	}

	void handle_handshakeing(const boost::system::error_code& error)
	{	
		std::istream responseStream(&m_revecMsg);

		std::string temp;
		std::string httpVersion;
		responseStream >> httpVersion;
		unsigned int statusCode;
		responseStream >> statusCode;
		std::string statusMessage;
		std::getline(responseStream, statusMessage);

		std::string contentType;
		std::getline(responseStream, contentType);
		std::string connection;
		std::getline(responseStream, connection);
		std::string transferEncoding;
		std::getline(responseStream, transferEncoding);

		unsigned int number;
		responseStream >> number;//무슨 숫자지?
		std::getline(responseStream, temp);
		std::string parseData;
		std::getline(responseStream, parseData);
	
		if(!responseStream || httpVersion.substr(0,5) != "HTTP/")
		{
			return;
		}
		if( statusCode != 200)
		{
			return;
		}
		
		// m_sessionID 뽑기
		boost::regex handshakeData("\"(.+?):(.+?):(.+?):");
		boost::match_results<const char*> resultHandshakeData;
		if( !boost::regex_search(parseData.c_str(), resultHandshakeData, handshakeData) )
		{
			// Connection 실패!
			return;
		}

		m_sessionID = resultHandshakeData.str(1);
		m_heartbeat = resultHandshakeData.str(2);
		m_timeout = resultHandshakeData.str(3);	

		// 데이터 다 빼기, 다른 방법이 있을까?!
		while( !responseStream.eof())
		{
			std::getline(responseStream, temp);
		}

		handle_wsConnect();
	}

	void handle_wsConnect()
	{
		std::string strHead2("Upgrade: websocket\n"
			"Connection: Upgrade\n"
			"Host: localhost:8080\n"
			"Origin: http://127.0.0.1:8080\n"		
			"Sec-WebSocket-Key: MTIzNDU2Nzg5MDEyMzQ1Ng==\n"		
			"Sec-WebSocket-Version: 13\n"
			"Sec-WebSocket-Extensions: x-webkit-deflate-frame\n\n");

		char head[1024];
		sprintf_s(head, "GET /socket.io/1/websocket/%s HTTP/1.1\n%s\n", m_sessionID.c_str(), strHead2.c_str());
		
		boost::asio::async_write(m_socket,
			boost::asio::buffer(head, strlen(head)),
			boost::bind(&CClientNet::handle_waite_connection, this,
			boost::asio::placeholders::error));
	}

	void handle_waite_connection(const boost::system::error_code& error)
	{
		boost::asio::async_read(m_socket, m_revecMsg, 
			boost::asio::transfer_at_least(1),
			boost::bind(&CClientNet::handle_ws_connection, this,
			boost::asio::placeholders::error));
	}

	void handle_ws_connection(const boost::system::error_code& error)
	{
		std::istream responseStream(&m_revecMsg);

		std::string temp;
		std::string httpVersion;
		responseStream >> httpVersion;
		unsigned int statusCode;
		responseStream >> statusCode;
		std::string statusMessage;
		std::getline(responseStream, statusMessage);

		std::string upgrade;
		std::getline(responseStream, upgrade);
		std::string connection;
		std::getline(responseStream, connection);
		std::string secWebSocketAccept;
		std::getline(responseStream, secWebSocketAccept);

		// 남은 데이터 다 빼기, 다른 방법이 있을까?!
		while( !responseStream.eof())
		{
			std::getline(responseStream, temp);
		}

		boost::asio::async_read(m_socket, m_revecMsg, 
			boost::asio::transfer_at_least(1),
			boost::bind(&CClientNet::handle_recv_data, this,
			boost::asio::placeholders::error));
	}

	void handle_recv_data(const boost::system::error_code& error)
	{
		std::istream recvStream(&m_revecMsg);

		//테스트용:받은 데이터를 전부 빼고
		std::string temp;
		while( !recvStream.eof())
		{
			std::getline(recvStream, temp);
		}

		// 서버에서 끊었다
		if(temp.empty())
		{
			m_socket.close();
			std::cout << "DISCONNECT!!";
			return;
		}

		// 다시 Recv 대기
		boost::asio::async_read(m_socket, m_revecMsg, 
			boost::asio::transfer_at_least(1),
			boost::bind(&CClientNet::handle_recv_data, this,
			boost::asio::placeholders::error));
	}

	void do_write(chat_message msg)
	{
		bool write_in_progress = !m_writeMsgQueue.empty();
		m_writeMsgQueue.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(m_socket,
				boost::asio::buffer(m_writeMsgQueue.front().data(),
				m_writeMsgQueue.front().length()),
				boost::bind(&CClientNet::handle_write, this,
				boost::asio::placeholders::error));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			m_writeMsgQueue.pop_front();
			if (!m_writeMsgQueue.empty())
			{
				boost::asio::async_write(m_socket,
					boost::asio::buffer(m_writeMsgQueue.front().data(),
					m_writeMsgQueue.front().length()),
					boost::bind(&CClientNet::handle_write, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			do_close();
		}
	}

	void do_close()
	{
		std::string payload("0:::");
		char frame[131]={0,};
		frame[0] = '\x81';
		frame[1] = 128 + payload.length();
		frame[2] = '\x00';
		frame[3] = '\x00';
		frame[4] = '\x00';
		frame[5] = '\x00';
		_snprintf(frame+6, 125, "%s", payload.c_str());

		boost::asio::async_write(m_socket,
			boost::asio::buffer(frame, payload.length() + 6),
			boost::bind(&CClientNet::handle_recv_data, this,
			boost::asio::placeholders::error));

		m_socket.close();
	}

private:
	boost::asio::io_service& m_ioService;
	tcp::socket m_socket;

	chat_message_queue m_writeMsgQueue;
	boost::asio::streambuf m_revecMsg;
	
	std::string m_sessionID;
	std::string m_heartbeat;
	std::string m_timeout;

};