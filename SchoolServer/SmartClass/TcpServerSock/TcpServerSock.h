#ifndef __SUNWOO_TCPSERVERSOCK_H
#define __SUNWOO_TCPSERVERSOCK_H

#include "Decl.h"

class TcpServerSock {
private:
	SOCKET		ServerSocket;
	SOCKET		ClientSocket;
	sockaddr_in	ServerAddr;
	sockaddr_in ClientAddr;
	socklen_t	ServerAddrSize;
	int port;
public:
	// Exception Class
	class Exception {
	private:
		int ErrorCode;
	public:
		Exception(const int error) : ErrorCode(error) { }
		const int GetErrorCode() const { return ErrorCode; }
		void What() {
			switch (ErrorCode) {
			case ERRLIST::SOCKERR:		// socket() function error
				std::cerr << "socket() error" << std::endl; break;
			case ERRLIST::BINDERR:		// bind() function error
				std::cerr << "bind() error" << std::endl; break;
			case ERRLIST::LISTENERR:	// listen() function error
				std::cerr << "listen() error" << std::endl; break;
			case ERRLIST::SOCKOPTERR:	// setsockopt() function error
				std::cerr << "setsockopt() error" << std::endl; break;
			case ERRLIST::ACCEPTERR:	// accept() function error
				std::cerr << "accept() error" << std::endl; break;
			case ERRLIST::SENDERR:		// send() function error
				std::cerr << "send() error" << std::endl; break;
			case ERRLIST::RECVERR:		// recv() function error
				std::cerr << "recv() error" << std::endl; break;
			default:
				std::cerr << "Unknown error" << std::endl;
			}
		}
	};

public:
	// default port number is 8000
	TcpServerSock(const int pt = 8000) throw(Exception);
	~TcpServerSock();
	
	SOCKET accept() throw(Exception);
	
	/*Send*/
	// Send to ClientSocket(member variable)
	int send(void *, const size_t) throw(Exception);
	int send(const void *, const size_t) throw(Exception);
	// Send to specified client
	int send(void *, const size_t, const SOCKET) throw(Exception);
	int send(const void *, const size_t, const SOCKET) throw(Exception);

	/*Receive*/
	// Receive from ClientSocket(member variable)
	int recv(void *, const size_t) throw(Exception);
	// Receive from specifed client
	int recv(void *, const size_t, const SOCKET) throw(Exception);
	
	SOCKET GetServerFD() const;
	SOCKET GetClientFD() const;
};
#endif
