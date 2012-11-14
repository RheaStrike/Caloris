#pragma once
#include "JSONCPP/json.h"
#include <regex>

class CEventSock;
class CSocketIOPayloader;


//typedef struct _packet
class packet
{
	friend class CSocketIOPayloader;
public:

	enum packetType
	{
		disconnect = 0,
		connect,
		heartbeat,
		message,	
		json,
		event,
		ack,
		error,
		noop,
	};

	packet(){}
	packet(packetType type) : m_Type(type){};
	//_packet(CalorisEvnet eventNum, unsigned char ssn, unsigned int usn)
	packet( unsigned char ssn, unsigned int usn):m_Type(packet::event)
	{

	}
	
//protected:
	packetType m_Type;
	string m_strEvent;
	Json::Value m_data;

};//packet;

class login : public packet
{

};

class JsonParser
{
public:
	JsonParser(){;}
	virtual ~JsonParser(){;}

	bool Get( const Json::Value& jsonData, const char* key, int& value)
	{
		const Json::Value& data = jsonData[key];
		if( !data.isString() )
		{
			return false;
		}
		value = ::atoi(data.asCString());
		return true;
	}

	bool Get( const Json::Value& jsonData, const char* key, string& value)
	{
		const Json::Value& data = jsonData[key];
		if( !data.isString() )
		{
			return false;
		}

		value.clear();
		value = data.asString();
		return true;
	}

	bool Get( const Json::Value& jsonData, const char* key, Json::Value& value)
	{
		const Json::Value& data = jsonData[key];
		if( !data.isObject() )
		{
			return false;
		}

		value.clear();
		value = data;
		return true;
	}
};

class CSocketIOPayloader
{
public:
	CSocketIOPayloader(void);
	virtual ~CSocketIOPayloader(void);

public:
	string	EncodePacket(packet& encodeData);
	string	EncodePacket(Json::Value& encodeData,  packet::packetType type = packet::event );
	bool	ProcessPacket(CEventSock* pSock, packet& recvPack);
	bool	ProcessPacket(string& recvData);

private:
	int		Unpack(string mask);

	void	Frame( BYTE opcode, string& data);
	bool	DecodePacket(char* recvData, packet& recvPack);
	bool	DecodePacket(string& recvData);

private:
	Json::StyledWriter	m_writer;
	Json::Reader		m_reader;
	std::basic_regex<char> m_rgxDecode;
};
