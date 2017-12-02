#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;

using namespace std;

const char MESSAGE_FLAG = '0';
const char LOGIN_FLAG = '1';
const char LOGIN_FAIL = 0;
const char LOGIN_SUCCESS = 1;
const int IDSIZE = 16;
const int PWSIZE = 16;
const int CARIDSIZE = 16;
const int LOCSIZE = 2;

typedef struct {
	char flag;
	char status;
	char id[CARIDSIZE + 1];
	char location[LOCSIZE + 1];
} packet_message;

typedef struct {
	char flag;
	char id[IDSIZE + 1];
	char pw[PWSIZE + 1];
} packet_login;

typedef struct {
	char id[CARIDSIZE + 1];
	int fee;
} packet_fee;

int main(void) {
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(8002);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	connect(client, (sockaddr *)&ServerAddr, sizeof(ServerAddr));

	char type;
	cout << "Enter data type (0 or 1) : "; cin >> type;
	if (type == MESSAGE_FLAG) {
		packet_message packet;
		packet.flag = static_cast<char>(type);
		cout << "Enter Car ID : "; cin >> packet.id;;
		cout << "Enter Car Status (0, 1) : "; cin >> packet.status;
		cout << "Enter Location (2byte) : "; cin >> packet.location;
	
		send(client, &packet, sizeof(packet), 0);

		if (packet.status == '0') {
			packet_fee packetFee;
			memset(&packetFee, 0, sizeof(packetFee));
			recv(client, &packetFee, sizeof(packetFee), 0);
	
			cout << endl;
			cout << "ID: " << packetFee.id << endl;
			cout << "Fee: " << packetFee.fee << endl;
			cout << endl;
		}
		
		close(client);
	}
	else if (type == LOGIN_FLAG){
		packet_login packet;
		packet.flag = type;
		cout << "Enter your ID : "; cin >> packet.id;
		cout << "Enter your PW : "; cin >> packet.pw;

		send(client, &packet, sizeof(packet), 0);
//		send(client, &packet.flag, sizeof(packet.flag), 0); 
//		send(client, packet.id, sizeof(packet.id), 0);
//		send(client, packet.pw, sizeof(packet.pw), 0);

		char res = 0;
		char name[16];
		recv(client, &res, sizeof(res), 0);
		if (res == LOGIN_FAIL)
			cout << "Login failed!" << endl;
		else if (res == LOGIN_SUCCESS)
			cout << "Login Success!" << endl;
		close(client);
	}

}
