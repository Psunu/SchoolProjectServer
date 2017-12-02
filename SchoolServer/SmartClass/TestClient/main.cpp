#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;

using namespace std;

const int STUDENTIDSIZE = 4;
const int NAMESIZE = 9;
const int IDSIZE = 16;
const int PWSIZE = 16;

// Device Type
const char RASP = '0';
const char ANDROID = '1';

// Flag
const char COMMON = '1';
const char LOGIN = '2';

// Common packet structure
typedef struct {
	char flag;
	char id[STUDENTIDSIZE + 1];
	char status;
	char reason;
	char week;
	char period;
} packetCommon;

// Login packet structure
typedef struct {
	char flag;
	char id[IDSIZE + 1];
	char password[PWSIZE + 1];
} packetLogin;

// Login success structure
typedef struct {
	char flag;
	char id[STUDENTIDSIZE + 1];
} packetLoginSuccess;

int main(void) {
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(8001);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	char type;
	cout << "Enter data type : "; cin >> type;
	if (type == COMMON) {
		int ClntNum = 0;
		cout << "Enter number of clients : "; cin >> ClntNum;

		SOCKET *clients = new SOCKET[ClntNum];
		packetCommon packet;
		memset(&packet, 0, sizeof(packet));
		packet.flag = COMMON;
		strcpy(packet.id, "3108");
		packet.status = '1';
		packet.reason = '1';
		packet.week = '4';
		packet.period = '1';
	
		for (int i = 0; i < ClntNum; i++) {
			clients[i] = socket(AF_INET, SOCK_STREAM, 0);
			connect(clients[i], (sockaddr *)&ServerAddr, sizeof(ServerAddr));
		
			printf("%s\n", packet.id);
	
			if (i == 0)
				send(clients[i], &RASP, sizeof(RASP), 0);
			else
				send(clients[i], &ANDROID, sizeof(ANDROID), 0);
	
			int res;
			send(clients[i], &packet.flag, sizeof(packet.flag), 0);
			send(clients[i], packet.id, sizeof(packet.id), 0);
			send(clients[i], &packet.status, sizeof(packet.status), 0);
			send(clients[i], &packet.reason, sizeof(packet.reason), 0);
			send(clients[i], &packet.week, sizeof(packet.week), 0);
			send(clients[i], &packet.period, sizeof(packet.period), 0);
//			res = send(clients[i], &packet, sizeof(packet), 0);
//			std::cout << "Send Size: " << res << std::endl;
			sleep(1);
		}
	
		for (int i = 0; i < ClntNum; i++) {
			close(clients[i]);
		}
	}
	else if (type == LOGIN){
		packetLogin packet;
		memset(&packet, 0, sizeof(packet));
		packet.flag = LOGIN;
		cout << "Enter your ID : "; cin >> packet.id;
		cout << "Enter your PW : "; cin >> packet.password;

		SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
		connect(client, (sockaddr *)&ServerAddr, sizeof(ServerAddr));
		send(client, &ANDROID, sizeof(RASP), 0);

		send(client, &packet, sizeof(packet), 0);

		packetLoginSuccess res_packet;
		memset(&res_packet, 0, sizeof(res_packet));
		recv(client, &res_packet, sizeof(res_packet), 0);
		if (res_packet.flag == 0)
			cout << "Login failed!" << endl;
		else {
			cout << "Login Success!" << endl;
			cout << "Name : " << packet.id << endl;
		}

		close(client);
	}

}
