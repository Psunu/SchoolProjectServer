#include <iostream>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>

typedef int SOCKET;

typedef struct {
	char Class;
	char Device;
	char Status;
} DevicePacket;

using namespace std;

int main(void) {
	SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(8000);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	connect(ClientSocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr));

	DevicePacket devPacket;
	char OTP[6];

	while (true) {
		char type = 0;
		cout << "Enter Packet Type! (0 = OTP, 1 = DEVICE, 2 = ANDROID) : "; scanf("%d", &type);
		send(ClientSocket, &type, sizeof(type), 0);		

		if (type == 0) {
			cout << "Enter OTP value : "; cin >> OTP;
			send(ClientSocket, OTP, sizeof(OTP), 0);
			memset(OTP, 0, sizeof(OTP));
		}
		else if (type == 1) {
			cout << "Enter Class (1 ~ 7) : "; scanf("%d", &devPacket.Class);
			cout << "Enter Device(0 ~ 2) : "; scanf("%d", &devPacket.Device);
			cout << "Enter Status(0 ~ 4) : "; scanf("%d", &devPacket.Status);
			send(ClientSocket, &devPacket, sizeof(devPacket), 0);
			memset(&devPacket, 0, sizeof(devPacket));
		}
	}
}
