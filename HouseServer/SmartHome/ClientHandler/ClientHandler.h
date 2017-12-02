#ifndef __SUNWOO_CLIENTHANDLER_H
#define __SUNWOO_CLIENTHANDLER_H

#include <iostream>
#include <cstring>

typedef int SOCKET;

class ClientHandler {
private:
	SOCKET *ClientList;
	int MaxClnt;	// Max number of clients
	int ClntNum;	// Current number of clients

public:
	ClientHandler(int maxclnt = 1024);
	ClientHandler(ClientHandler&);
	~ClientHandler();
	
	ClientHandler& operator=(ClientHandler&);
	SOCKET operator[](int) const;
	
	int GetClntNum() const;
	int GetMaxClnt() const;
	int AddClnt(SOCKET);
	int RemoveClnt(SOCKET);
};
#endif
