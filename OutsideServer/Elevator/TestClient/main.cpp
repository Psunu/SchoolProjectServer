#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;

using namespace std;

const char FLOOR_FLAG = '0';
const char BUTTON_FLAG = '1';
const char LOGIN_FLAG = '2';

const char LOGIN_FAIL = '0';
const char LOGIN_SUCCESS = '1';

const int IDSIZE = 16;
const int PWSIZE = 16;

typedef struct {
	char flag;
	char id[IDSIZE + 1];
	char pw[PWSIZE + 1];
} packet_login;

int main(void) {
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(8001);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	connect(client, (sockaddr *)&ServerAddr, sizeof(ServerAddr));

	char type;
	cout << "Enter data type (0 ~ 2) : "; cin >> type;
	if (type == FLOOR_FLAG) {
		char floor;
		cout << "Enter floor : "; cin >> floor;
		floor -= '0';
	
		send(client, &FLOOR_FLAG, sizeof(FLOOR_FLAG), 0);
		send(client, &floor, sizeof(floor), 0);
	}
	else if (type == BUTTON_FLAG) {
		char button;
		cout << "Enter button (0 or 1) : "; cin >> button;
		button -= '0';

		send(client, &BUTTON_FLAG, sizeof(BUTTON_FLAG), 0);
		send(client, &button, sizeof(button), 0);
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
	}

	close(client);
}
