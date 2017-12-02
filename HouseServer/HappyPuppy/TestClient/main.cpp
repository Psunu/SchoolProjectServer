#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

typedef int SOCKET;
typedef struct {
	char feed1_amount;
	char feed2_amount;
	char feed3_amount;
	char water_amount;
} packetShowAmount;

typedef struct {
	char feed;
	char feed_amount;
} packetFeed;

enum {
	SHOW_AMOUNT,
	SHOW_PICTURE,
	FEED
};

enum {
	FEED1,
	FEED2,
	FEED3,
	WATER
};

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
	type -= '0';
	if (type == SHOW_AMOUNT) {
		packetShowAmount packet;
		memset(&packet, 0, sizeof(packet));

		std::cout << "Enter amount of feed1 (0 ~ 2) : "; std::cin >> packet.feed1_amount;
		std::cout << "Enter amount of feed2 (0 ~ 2) : "; std::cin >> packet.feed2_amount;
		std::cout << "Enter amount of feed3 (0 ~ 2) : "; std::cin >> packet.feed3_amount;
		std::cout << "Enter amount of water (0 ~ 2) : "; std::cin >> packet.water_amount;
		packet.feed1_amount -= '0';
		packet.feed2_amount -= '0';
		packet.feed3_amount -= '0';
		packet.water_amount -= '0';

		send(client, &type, sizeof(type), 0);
		send(client, &packet.feed1_amount, sizeof(packet.feed1_amount), 0);
		send(client, &packet.feed2_amount, sizeof(packet.feed2_amount), 0);
		send(client, &packet.feed3_amount, sizeof(packet.feed3_amount), 0);
		send(client, &packet.water_amount, sizeof(packet.water_amount), 0);
	}
	else if (type == SHOW_PICTURE){
		send(client, &type, sizeof(char), 0);
	}
	else if (type == FEED) {
		packetFeed packet;
		memset(&packet, 0, sizeof(packet));

		std::cout << "Enter type of feed (0 ~ 3) : "; std::cin >> packet.feed;
		std::cout << "Enter amount of feed (0 ~ 2) : "; std::cin >> packet.feed_amount;
		packet.feed -= '0';
		packet.feed_amount -= '0';

		send(client, &type, sizeof(char), 0);
		send(client, &packet.feed, sizeof(packet.feed), 0);
		send(client, &packet.feed_amount, sizeof(packet.feed_amount), 0);
	}

	close(client);
}
