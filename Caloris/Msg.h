#pragma once

#include "../Include/jsoncpp/reader.h"
#include "../Include/jsoncpp/writer.h"

#pragma comment(lib,"../Lib/jsoncpp/json_vc71_libmtd.lib")

typedef int MsgID;

class CMsg
{
public:
	enum { header_length = 6 };
	enum { max_body_length = 125 };

	CMsg(){;}
	virtual ~CMsg(){;}

public:
	char*	GetBuffer(){return m_data;}
	size_t	GetWriteLength(){ return m_writeLength; }

public:
private:
	char	m_data[max_body_length];
	size_t	m_writeLength;

	Json::Value	m_value;	
};


/*

bool	m_bLastFragment;
bool	m_bMasked;
BYTE	m_opcode;
BYTE	m_activeFragmentedOperation;

BYTE	m_packetType;

void	encode_header()
{
}

bool	decode_header()
{
const char& frame1 = m_data[0];
const char& frame2 = m_data[1];

m_bLastFragment	= (frame1 & 0x80) == 0x80;
m_bMasked		= (frame2 & 0x80) == 0x80;

BYTE opcode = frame1 & 0xf;
if( opcode == 0 )
{
m_opcode = m_activeFragmentedOperation;
if( !(m_opcode == 1 || m_opcode ==2) )
{
printf("continuation frame cannot follow current opcode\n");
return false;
}
}
else
{
m_opcode = opcode;
if( m_bLastFragment == false )
{
m_activeFragmentedOperation = opcode;
}
}

switch( m_opcode)
{
case 1:	// text
{

}break;
case 8: // close
{

}break;
case 9:	//ping
{

}break;
default:
{
printf("no handler for opcode : %d", m_opcode);
return false;
}
}

return true;
}

bool	encode_packet()
{
return true;
}
*/