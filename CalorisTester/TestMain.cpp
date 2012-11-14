
#pragma comment(lib,"../../ClientLib/CalorisD.lib")
#include "../Caloris/ClientNet.h"


int main(int argc, char* argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: chat_client <host> <port>\n";
			return 1;
		}

		/////////////////////////////////////////////////////////////////
		char szID[16]={0,};
		std::cout << "ID : ";
		std::cin.getline(szID, 16);
		char szUSN[3]={0,};
		std::cout << "USN(number): ";
		std::cin.getline(szUSN, 3);
		
		//Json::Value userData;
		//userData["name"] = "userData";
		//userData["args"]["name"] = "abcd";
		//userData["args"]["usn"] = "11";
		//Json::StyledWriter writer;
		//std::string strUserData = userData.toStyledString();

		char userData[110]={0,};//{"5:::{\"name\":\"userData\",\"args\":[\"{ \\\"name\\\":\\\"abcd\\\",\\\"usn\\\":\\\"5\\\"}\"]}"};//
		_snprintf(userData, 110, "5:::{\"name\":\"userData\",\"args\":[\"{ \\\"name\\\":\\\"%s\\\",\\\"usn\\\":\\\"%s\\\"}\"]}", szID, szUSN);
								 
		char frame1[131]={0,};
		frame1[0] = '\x81';
		frame1[1] = 128 + strlen(userData);
		frame1[2] = '\x00';
		frame1[3] = '\x00';
		frame1[4] = '\x00';
		frame1[5] = '\x00';
		_snprintf(frame1+6, 125, "%s", userData);
		/////////////////////////////////////////////////////////////////

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], argv[2]);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		CClientNet c(io_service, iterator);

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
		

		char heartbeat[]={"2:::"};
		char frame[131]={0,};
		frame[0] = '\x81';
		frame[1] = 128 + strlen(heartbeat);
		frame[2] = '\x00';
		frame[3] = '\x00';
		frame[4] = '\x00';
		frame[5] = '\x00';
		_snprintf(frame+6, 125, "%s", heartbeat);

		char line[chat_message::max_body_length + 1];
		while(std::cin.getline(line, chat_message::max_body_length + 1))
		{
			using namespace std; // For strlen and memcpy.
			/////////////////////////////////////////////////////////////////
			chat_message msg;
			msg.length(strlen(heartbeat)+6);
			memcpy(msg.data(), frame, msg.length());
			//msg.encode_header();
			c.write(msg);
			/////////////////////////////////////////////////////////////////
			chat_message msg1;
			msg1.length(strlen(userData)+6);
			memcpy(msg1.data(), frame1, msg1.length());
			//msg.encode_header();
			c.write(msg1);
			/////////////////////////////////////////////////////////////////

		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}