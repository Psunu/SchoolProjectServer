#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;

using namespace std;

const char RASP = '0';
const char ANDROID = '1';
char name[10] = "박선우";
char status = '4';


typedef struct {
	char name[10];
	char status;
} Packet;

int main(void) {
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(8000);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	int type;
	cout << "Enter data type : "; cin >> type;
	if (type == 1) {
		int ClntNum = 0;
		cout << "Enter number of clients : "; cin >> ClntNum;
		SOCKET *clients = new SOCKET[ClntNum];
	
		for (int i = 0; i < ClntNum; i++) {
			clients[i] = socket(AF_INET, SOCK_STREAM, 0);
			connect(clients[i], (sockaddr *)&ServerAddr, sizeof(ServerAddr));
		
			printf("%s\n", name);
	
			if (i == 0)
				send(clients[i], &RASP, sizeof(RASP), 0);
			else
				send(clients[i], &ANDROID, sizeof(ANDROID), 0);
	
			Packet pakcet;
			memset(&packet, 0, sizeof(packet));
			char flag = '1';
			send(clients[i], &flag, sizeof(flag), 0);
			send(clients[i], name, 9, 0);
			send(clients[i], &status, sizeof(status), 0);
			sleep(1);
		}
	
		for (int i = 1; i < ClntNum; i++) {
			Packet packet; memset(&packet, 0, sizeof(packet));
			recv(clients[0], packet.name, 9, 0);
			recv(clients[0], &packet.status, sizeof(packet.status), 0);
			printf("Name: %s\n", packet.name);
			printf("Status: %d\n\n", packet.status);
		}
	
		for (int i = 0; i < ClntNum; i++) {
			close(clients[i]);
		}
	}
	else if (type == 2){
		char flag = '2';
		char id[16];
		char pass[16];

		cout << "Enter your ID : "; cin >> id;
		cout << "Enter your PW : "; cin >> pass;

		SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
		connect(client, (sockaddr *)&ServerAddr, sizeof(ServerAddr));
		send(client, &ANDROID, sizeof(RASP), 0);

		send(client, &flag, sizeof(flag), 0); 
		send(client, id, sizeof(id), 0);
		send(client, pass, sizeof(pass), 0);

		char res = 0;
		char name[16];
		recv(client, &res, sizeof(res), 0);
		if (res == 0)
			cout << "Login failed!" << endl;
		else {
			recv(client, name, sizeof(name), 0);
			cout << "Login Success!" << endl;
			cout << "Name : " << name << endl;
		}

		close(client);
	}

}
