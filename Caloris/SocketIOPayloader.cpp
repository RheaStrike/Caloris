#include "StdAfx.h"
#include "SocketIOPayloader.h"
#include "SocketUtil/EventSock.h"

CSocketIOPayloader::CSocketIOPayloader(void)
{
	m_rgxDecode = "([^:]+):([0-9]+)?[\+]?:([^:]+)?:?(.+)?";
}


CSocketIOPayloader::~CSocketIOPayloader(void)
{
}


string CSocketIOPayloader::EncodePacket( packet& encodeData )
{
	string	strData;
	string	encoded;
	switch(encodeData.m_Type)
	{
	case packet::message:
		{
			if(!encodeData.m_data.empty())
			{
				strData = m_writer.write(encodeData.m_data);
			}
		}break;
	case packet::event:
		{
			if(!encodeData.m_strEvent.empty()
				&& !encodeData.m_data.empty())
			{
				Json::Value temp;
				Json::StyledWriter writer;
				temp["name"] = encodeData.m_strEvent.c_str();
				temp["args"] = writer.write(encodeData.m_data);

				strData = writer.write(temp);
			}
		}break;
	default:
		{
			return encoded;//empty;
		}break;
	}

	char szEncoded[10] = {0,};
	sprintf_s(szEncoded,"%d:::",encodeData.m_Type);
	encoded.append(szEncoded);
	encoded.append(strData);

	Frame(0x81, encoded);

	return encoded;
}

std::string CSocketIOPayloader::EncodePacket( Json::Value& encodeData, packet::packetType type /*= packet::event */ )
{
	string	strData;
	string	encoded;
	switch(type)
	{
	case packet::message:
		{
			if(!encodeData.empty())
			{
				strData = m_writer.write(encodeData);
			}
		}break;
	case packet::event:
		{
			if(!encodeData["action"].empty()
				&& !encodeData.empty())
			{
				Json::Value temp;
				Json::StyledWriter writer;
				temp["name"] = encodeData["action"];
				temp["args"] = writer.write(encodeData);

				strData = writer.write(temp);
			}
		}break;
	default:
		{
			return encoded;//empty;
		}break;
	}

	char szEncoded[10] = {0,};
	sprintf_s(szEncoded,"%d:::",type);
	encoded.append(szEncoded);
	encoded.append(strData);

	Frame(0x81, encoded);

	return encoded;
}


void CSocketIOPayloader::Frame( BYTE opcode, string& data )
{
	int startOffset = 2;
	size_t dataLength = data.length();
	size_t secondByte;

	if(dataLength > 65536)
	{
		startOffset = 10;
		secondByte = 127;
	}
	else if(dataLength>125)
	{
		startOffset = 4;
		secondByte =126;
	}
	else
	{
		secondByte = dataLength;
	}

	char frame[8] = {0,};

	frame[0] = opcode;
	frame[1] = secondByte;

	switch(secondByte)
	{
	case 126:
		{
			frame[2] = dataLength>>8;
			frame[3] = dataLength%256;
		}break;
	case 127:
		{
			size_t temp = dataLength;
			for( int i = 0; i<8; ++i)
			{
				frame[startOffset-1] = temp & 0xff;
				temp = temp >> 8; // temp >>>= 8
			}
		}
	}

	string packMsg;
	packMsg.append(frame,startOffset);
	packMsg.append(data);
	
	data.swap(packMsg);
}


int CSocketIOPayloader::Unpack(string mask)
{
	int unpack = 0;
	for(size_t i = 0; i < mask.length(); ++i)
	{
		unsigned char temp = mask[i];
		unpack = (i == 0) ? temp : (unpack*256) + temp;
	}
	return unpack;
}


bool CSocketIOPayloader::ProcessPacket(CEventSock* pSock, packet& recvPack)
{
	char szMask[8] = {0,};
	int nReceive = pSock->Receive(szMask, 2);
	if((szMask[0] & 0x70) != 0)
	{
		return false;
	}

	size_t dataLength = szMask[1] & 0x7f;
	if(dataLength < 126)
	{
		//nothing
	}
	else if(dataLength == 126)
	{
		nReceive = pSock->Receive(szMask, 2);
		szMask[nReceive] ='\x0';
		dataLength = Unpack(string(szMask, nReceive));
	}
	else if(dataLength == 127)
	{
		nReceive = pSock->Receive(szMask, 8) ;
		szMask[nReceive] ='\x0';
		if(Unpack(string(szMask,0, 4))!=0)
		{
			return false;
		}
		dataLength = Unpack(string(szMask, 4, 4));
	}
	else
	{
		return false;
	}

	char* szRecvData = new char[dataLength];
	nReceive = pSock->Receive( szRecvData,dataLength);
	if( nReceive != dataLength)
	{
		return false;
	}

	bool bResult = DecodePacket(szRecvData, recvPack);

	delete[] szRecvData;

	return bResult;
}


bool CSocketIOPayloader::DecodePacket(char* recvData, packet& recvPack)
{
	std::match_results<const char*> results;
	std::regex_search( recvData, results, m_rgxDecode);

	if(results.empty())
	{
		return false;
	}

	Json::Value root;
	string temp = results[1];
	int packType = atoi(temp .c_str());

	switch(packType)
	{
	case packet::heartbeat:
		{
			recvPack.m_Type = packet::heartbeat;
		}break;
	case packet::message:
		{
			recvPack.m_Type = packet::message;

			if(false == m_reader.parse(results[4], root))
			{
				return false;
			}
			recvPack.m_data = root;
		}break;
	case packet::event:
		{
			recvPack.m_Type = packet::event;

			if(false == m_reader.parse(results[4], root))
			{
				return false;
			}
			recvPack.m_strEvent = m_writer.write(root.get("name", Json::Value::null));
			const Json::Value& args = root.get("args", Json::Value::null);

			int index = 0;
			if( args != Json::Value::null )
			{
				 m_reader.parse(args[index].asString(), recvPack.m_data);
			}

		}break;
	default:
		{
			return false;
		}
	}

	return true;
}


bool CSocketIOPayloader::ProcessPacket(string& recvData)
{
	if((recvData[0] & 0x70) != 0)
	{
		return false;
	}

	string data;
	int dataLength = recvData[1] & 0x7f;
	if(dataLength < 126)
	{
		data.append(recvData, 2, dataLength);
	}
	else if(dataLength == 126)
	{
		dataLength = Unpack(string(recvData,2, 2));
		data.append(recvData, 4, dataLength);
	}
	else if(dataLength == 127)
	{
		if(Unpack(string(recvData,0, 4))!=0)
		{
			return false;
		}
		dataLength = Unpack(string(recvData, 4, 4));
		data.append(recvData, 8, dataLength);
	}
	else
	{
		return false;
	}

	if(DecodePacket(data))
	{
		recvData.swap(data);
		return true;
	}
	return false;
}


bool CSocketIOPayloader::DecodePacket(string& recvData)
{
	std::match_results<const char*> results;
	std::regex_search( recvData.c_str(), results, m_rgxDecode);

	if(results.empty())
	{
		return false;
	}

	string temp = results[4];

	if(temp.empty())
	{
		return false;
	}

	temp.swap(recvData);
	return true;
}