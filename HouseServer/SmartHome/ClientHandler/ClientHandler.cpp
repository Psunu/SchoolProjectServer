#include "ClientHandler.h"

ClientHandler::ClientHandler(int maxclnt) : ClntNum(0) {
	MaxClnt = maxclnt;
	ClientList = new SOCKET[MaxClnt];
	memset(ClientList, 0, MaxClnt);
}

ClientHandler::ClientHandler(ClientHandler &ClntHand) {
	MaxClnt = ClntHand.MaxClnt;
	ClntNum = ClntHand.ClntNum;
	ClientList = new SOCKET[MaxClnt];
	memset(ClientList, 0, MaxClnt);

	for (int i = 0; i < ClntNum; i++) {
		ClientList[i] = ClntHand.ClientList[i];
	}
}

ClientHandler::~ClientHandler() {
	delete[] ClientList;
}

ClientHandler& ClientHandler::operator=(ClientHandler &ClntHand) {
	MaxClnt = ClntHand.MaxClnt;
	ClntNum = ClntHand.ClntNum;
	
	delete[] ClientList;
	ClientList = new SOCKET[MaxClnt];
	memset(ClientList, 0, MaxClnt);

	for (int i = 0; i < ClntNum; i++) {
		ClientList[i] = ClntHand.ClientList[i];
	}
}

SOCKET ClientHandler::operator[](int index) const {
	return ClientList[index];
}

int ClientHandler::GetClntNum() const { return ClntNum; };
int ClientHandler::GetMaxClnt() const { return MaxClnt; };
int ClientHandler::AddClnt(SOCKET sock) { ClientList[sock] = sock; };
int ClientHandler::RemoveClnt(SOCKET sock) { ClientList[sock] = 0; };
