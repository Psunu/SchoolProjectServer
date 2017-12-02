#ifndef __SUNWOO_HANDLER_H
#define __SUNWOO_HANDLER_H

#include <iostream>
#include <cstdio>
#include <cstdio>
#include <cstring>
// MySQL Header
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
// Custom Header
#include "TcpServerSock/TcpServerSock.h"
#include "Time/Time.h"
#include "Decl.h"

class Handler {
private:
	// Socket
	TcpServerSock	*Server;
	SOCKET		 	ServerSocket;
	SOCKET			RaspSocket;

	// MySQL
	sql::Driver				*driver;
	sql::Connection			*con;
	sql::PreparedStatement	*EventStmt;
	sql::PreparedStatement	*AmountStmt;

	// Select
	fd_set	reads, ori_reads;
	int		maxfd;

	// Time
	Time tm;
	char EventTime[32];

	// Power
	bool Power;

private:
	void InitSQL();
	void UpdateTime();

	char* ConvertAmount(const char);
	char* ConvertType(const char);

	void ProcNewConn();
	void ProcDisconn(SOCKET);
	void ProcShowAmount(SOCKET);
	void ProcFeed(SOCKET);
	void ProcShowPicture();

public:
	Handler();
	~Handler();

	void Run();
	void Stop();
	void Start();
};	
#endif
