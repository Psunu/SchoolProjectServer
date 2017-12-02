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

	// Select
	fd_set	reads, ori_reads;
	int		maxfd;

	// MySQL
	sql::Driver				*driver;
	sql::Connection			*con;
	sql::PreparedStatement	*EventStmt;
	sql::PreparedStatement	*LoginStmt;
	sql::PreparedStatement	*NameStmt;

	// Time
	Time tm;
	char EventTime[32];

	// Current DB Name
	char DBName[32];	
	// Current Period
	int CurPeriod;

	// Power
	bool Power;
	
private:
	// Initialize MySQL method
	void InitSQL();
	void ClearEtc();
	void MakeAttend();

	void UpdateTime();
	char* ConvertStatus(packetCommon&);
	char* ConvertCommon(packetCommon&);
	char* ConvertGoout(packetCommon&);
	char* ConvertEtc(packetCommon&);
	char* ConvertName(packetCommon&);
	char* ConvertWeek(packetCommon&);
	char* ConvertPeriod(packetCommon&);
	int ConvertPriority(packetCommon&);

	void ProcNewConn();
	void ProcDisconn(SOCKET);
	void ProcCommon(SOCKET);
	void ProcRaspberry(packetCommon&);
	void ProcAndroid(packetCommon&);
	void ProcLogin(SOCKET);
	bool CheckLogin(packetLogin&);
	void ProcForce(SOCKET);
	
	void RunAttendSQL(packetCommon&, char*, char*, char*);

public:
	Handler();
	~Handler();

	void Run();
	void Stop();
	void Start();
};	
#endif
