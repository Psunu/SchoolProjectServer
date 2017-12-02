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

	// MySQL
	sql::Driver				*driver;
	sql::Connection			*con;
	sql::PreparedStatement	*ControlRunStmt;
	sql::PreparedStatement	*ControlPushupStmt;
	sql::PreparedStatement	*ControlFlexStmt;
	sql::PreparedStatement	*CircuitRunStmt;
	sql::PreparedStatement	*CircuitPushupStmt;
	sql::PreparedStatement	*CircuitFlexStmt;
	sql::PreparedStatement	*CommRunStmt;
	sql::PreparedStatement	*CommPushupStmt;
	sql::PreparedStatement	*CommFlexStmt;
	sql::PreparedStatement	*LoginStmt;

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

	char* ConvertDept(packet_message&);
	char* ConvertRate(packet_message&);

	void ProcNewConn();
	void ProcDisconn(SOCKET);
	void ProcMessage(SOCKET);
	void ProcRun(packet_message&);
	void ProcPushup(packet_message&);
	void ProcFlex(packet_message&);
	void ProcLogin(SOCKET);
	bool CheckLogin(packet_login&);

public:
	Handler();
	~Handler();

	void Run();
	void Stop();
	void Start();
};	
#endif
