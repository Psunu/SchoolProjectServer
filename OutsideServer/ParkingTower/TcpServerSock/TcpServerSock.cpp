#include "TcpServerSock.h"

TcpServerSock::TcpServerSock(const int pt) throw(Exception)
	: port(pt), ClientSocket(0) {
	// Create socket
	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket == ERROR)
		throw Exception(ERRLIST::SOCKERR);

	ServerAddrSize = sizeof(ServerAddr);
	memset(&ServerAddr, 0, ServerAddrSize);
	// Server connection information
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(port);
	ServerAddr.sin_addr.s_addr	= htonl(INADDR_ANY);

	int res;
	int yes = 1;
	// Avoid socket bind error
	res = setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (res == ERROR)
		throw Exception(ERRLIST::SOCKOPTERR);
	
	// Binding
	res = bind(ServerSocket, reinterpret_cast<sockaddr *>(&ServerAddr), ServerAddrSize);
	if (res == ERROR)
		throw Exception(ERRLIST::BINDERR);

	// Listenning
	res = listen(ServerSocket, 10);
	if (res == ERROR)
		throw Exception(ERRLIST::LISTENERR);
}

TcpServerSock::~TcpServerSock() { close(ServerSocket); };

SOCKET TcpServerSock::accept() throw(Exception) {
	socklen_t ClientAddrSize;
	SOCKET temp = ClientSocket;
	memset(&ClientAddr, 0, sizeof(ClientAddr));

	// Accept
	ClientSocket = ::accept(ServerSocket, reinterpret_cast<sockaddr *>(&ClientAddr), &ClientAddrSize);
	if (ClientSocket == ERROR) {
		ClientSocket = temp;
		throw Exception(ERRLIST::ACCEPTERR);
	}

	// Return connected client socket
	return ClientSocket;
}

// Send to recent connected client
int TcpServerSock::send(void *buff, const size_t len) throw(Exception) {
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::send(ClientSocket, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::SENDERR);

	// Return sended data length
	return res;
}

int TcpServerSock::send(const void *buff, const size_t len) throw(Exception) {
	buff = const_cast<void *>(buff);
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::send(ClientSocket, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::SENDERR);

	// Return sended data length
	return res;
}

// Send to specific socket
int TcpServerSock::send(void *buff, const size_t len, const SOCKET sock) throw(Exception) {
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::send(sock, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::SENDERR);
	
	// Return sended data length
	return res;
}

int TcpServerSock::send(const void *buff, const size_t len, const SOCKET sock) throw(Exception) {
	buff = const_cast<void *>(buff);
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::send(sock, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::SENDERR);
	
	// Return sended data length
	return res;
}

// Receive from recent connected client
int TcpServerSock::recv(void *buff, const size_t len) throw(Exception) {
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::recv(ClientSocket, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::RECVERR);

	// Return received data length
	return res;
}

// Receive from specific socket
int TcpServerSock::recv(void *buff, const size_t len, const SOCKET sock) throw(Exception) {
	// MSG_NOSIGNAL option is to avoid program close
	int res = ::recv(sock, buff, len, MSG_NOSIGNAL);
	if (res == ERROR)
		throw Exception(ERRLIST::RECVERR);

	// Return received data length
	return res;
}

SOCKET TcpServerSock::GetServerFD() const { return ServerSocket; }
SOCKET TcpServerSock::GetClientFD() const { return ClientSocket; }
