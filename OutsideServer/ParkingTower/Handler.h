#ifndef __SUNWOO_HANDLER_H
#define __SUNWOO_HANDLER_H

#include <iostream>
#include <cstdio>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
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
	sql::PreparedStatement	*EventStmt;
	sql::PreparedStatement	*AddCarStmt;
	sql::PreparedStatement	*DelCarStmt;
	sql::PreparedStatement	*LoginStmt;

	// Select
	fd_set	reads, ori_reads;
	int		maxfd;

	// Time
	Time tm;
	char EventTime[32];

	// Map [CarID : Minute]
	std::map<std::string, int> CarList;

	// Power
	bool Power;

private:
	void InitSQL();
	void UpdateTime();

	void ProcNewConn();
	void ProcDisconn(SOCKET);
	void ProcMessage(SOCKET);
	void ProcIn(packet_message&);
	int ProcOut(packet_message&);
	void ProcLogin(SOCKET);

	bool CheckLogin(packet_login&);
	int CalcFee(packet_message&);

public:
	Handler();
	~Handler();

	void Run();
	void Stop();
	void Start();
};	
#endif
