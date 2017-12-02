#ifndef __SUNWOO_HANDLER_H
#define __SUNWOO_HANDLER_H

#include <iostream>
#include <cstdio>
#include <cstring>
// MySQL Header
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
// Custom Header
#include "ClientHandler/ClientHandler.h"
#include "TcpServerSock/TcpServerSock.h"
#include "Time/Time.h"
#include "Decl.h"

class Handler {
private:
	// Socket
	TcpServerSock	*Server;
	SOCKET			ServerSocket;
	// ClientHandler
	ClientHandler	*ClntHandler;
	// Select
	fd_set	reads, ori_reads;
	int		maxfd;
	// MySQL
	sql::Driver				*driver;
	sql::Connection			*con;
	sql::Statement			*InitStmt;
	sql::PreparedStatement	*EventStmt;
	sql::PreparedStatement	*CurrentStmt;
	sql::PreparedStatement	*OtpStmt;
	// Time
	Time tm;
	char EventTime[32];
	// Power
	bool Power;
	
private:
	void InitSQL();
	void UpdateTime();
	void OccurGas();
	char* ExtractStatus(Device&);
	char* ExtractClass(Device&);
	char* ExtractDevice(char);
	void ProcOTP(SOCKET);	
	void ProcDevice(SOCKET);
	void ProcNewConn();
	void ProcDisconn(SOCKET);

public:
	Handler();
	~Handler();

	void Run();
	void Stop();
	void Start();
};	
#endif
