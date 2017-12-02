#include "Handler.h"

Handler::Handler() : Power(true), RaspSocket(0), CurPeriod(0) {
	// Socket Setting
	Server = new TcpServerSock(8001);
	ServerSocket = Server->GetServerFD();
	
	// Select setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// Mysql connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("SmartClass");

	memset(DBName, 0, sizeof(DBName));
}

Handler::~Handler() {
	delete NameStmt;
	delete LoginStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, ID text, Status text, Period text,Date text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS StudentList(Number int, Name text, ID text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EtcList(Number int, Name text, Status text)");

	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(ID, Status, Period, Date) VALUES(?, ?, ?, ?)");
	LoginStmt = con->prepareStatement("SELECT ID, Password FROM UserInfo WHERE ID=?");
	NameStmt = con->prepareStatement("SELECT Name FROM StudentList WHERE ID=?");

	MakeAttend();
	delete InitStmt;
}

void Handler::ClearEtc() {
	std::cout << "ClearEtc()" << std::endl;
	sql::Statement *stmt = con->createStatement();
	try {
		stmt->execute("DROP TABLE IF EXISTS EtcList");
		stmt->execute("CREATE TABLE IF NOT EXISTS EtcList(Number int PRIMARY KEY, Name text, Status text)");
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ClearEtc(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
	delete stmt;
}

void Handler::MakeAttend() {
	std::cout << "MakeAttend()" << std::endl;
	char charDay[32] = {0, };
	sprintf(charDay, "%02d_%02d", tm.Get_mon(), tm.Get_day());
	int DBdate = (DBName[0] - '0') + (DBName[1] - '0') + (DBName[3] - '0') + (DBName[4] - '0');
	int CurDay = (charDay[0] - '0') + (charDay[1] - '0') + (charDay[3] - '0') + (charDay[4] - '0');

	if (CurDay != DBdate) {
		char query[256] = {0, };
	
		sprintf(DBName, "%02d_%02dAttendance", tm.Get_mon(), tm.Get_day());
		sprintf(query, "CREATE TABLE IF NOT EXISTS %s(Number int PRIMARY KEY, Name text, First text, Second text, Third text, Fourth text, Fifth text, Sixth text, Seventh text)", DBName);
	
		sql::Statement *stmt = con->createStatement();
		sql::ResultSet *res;
		try { 
			stmt->execute(query);
			res = stmt->executeQuery("SELECT Number, Name FROM StudentList");
			while (res->next()) {
				memset(query, 0, sizeof(query));
				int number = res->getInt("Number");
				char *name = const_cast<char*>(res->getString("Name").c_str());
				sprintf(query, "INSERT INTO %s(Number, Name) VALUES(%d, '%s')", DBName, number, name);
				stmt->execute(query);
			}
		} 
		catch (sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in MakeAttend(), SQL error" << std::endl;
			std::cerr << e.what();
			std::cerr << std::endl;
		}
		delete stmt;
	}
}

// Update Current Time
void Handler::UpdateTime() {
	memset(EventTime, 0, sizeof(EventTime));
	// Year-Month-Day-Hour-Minute
	sprintf(EventTime, "%d-%02d-%02d %02d:%02d", tm.Get_year(), tm.Get_mon(), tm.Get_day(), tm.Get_hour(), tm.Get_min());
}
		
// Convert status message to string
char* Handler::ConvertStatus(packetCommon &packet) {
	int len = 0;
	char *stat;

	switch (packet.status) {
	case MESSAGE::STATUS::LATENESS:
		return ConvertCommon(packet);
	case MESSAGE::STATUS::ABSENCE:
		return ConvertCommon(packet);
	case MESSAGE::STATUS::EARLYLEAVE:
		return ConvertCommon(packet);
	case MESSAGE::STATUS::GOOUT:
		return ConvertGoout(packet);
	case MESSAGE::STATUS::ETC:
		return ConvertEtc(packet);
	case MESSAGE::STATUS::NOCARD:
		len = sizeof("카드없음");
		stat = new char[len];
		strncpy(stat, "카드없음", len);
		return stat;
	case MESSAGE::STATUS::ATTEND:
		len = sizeof("출석");
		stat = new char[len];
		strncpy(stat, "출석", len);
		return stat;
	default:
		stat = new char[sizeof("Unknown")];	// Have to delete
		strcpy(stat, "Unknown");
		return stat;
	}
}

char* Handler::ConvertCommon(packetCommon &packet) {
	int len;
	char *stat;

	switch (packet.reason) {
	case MESSAGE::COMMON::DISEASE:
		if (packet.status == MESSAGE::STATUS::LATENESS) {
			len = sizeof("질병지각");
			stat = new char[len];
			strncpy(stat, "질병지각", len);
		}
		else if (packet.status == MESSAGE::STATUS::ABSENCE) {
			len = sizeof("질병결석");
			stat = new char[len];
			strncpy(stat, "질병결석", len);
		}
		else if (packet.status == MESSAGE::STATUS::EARLYLEAVE) {
			len = sizeof("질병조퇴");
			stat = new char[len];
			strncpy(stat, "질병조퇴", len);
		}
		break;
	case MESSAGE::COMMON::RECOGNITION:
		if (packet.status == MESSAGE::STATUS::LATENESS) {
			len = sizeof("인정지각");
			stat = new char[len];
			strncpy(stat, "인정지각", len);
		}
		else if (packet.status == MESSAGE::STATUS::ABSENCE) {
			len = sizeof("인정결석");
			stat = new char[len];
			strncpy(stat, "인정결석", len);
		}
		else if (packet.status == MESSAGE::STATUS::EARLYLEAVE) {
			len = sizeof("인정조퇴");
			stat = new char[len];
			strncpy(stat, "인정조퇴", len);
		}
		break;
	case MESSAGE::COMMON::WITHOUT:
		if (packet.status == MESSAGE::STATUS::LATENESS) {
			len = sizeof("무단지각");
			stat = new char[len];
			strncpy(stat, "무단지각", len);
		}
		else if (packet.status == MESSAGE::STATUS::ABSENCE) {
			len = sizeof("무단결석");
			stat = new char[len];
			strncpy(stat, "무단결석", len);
		}
		else if (packet.status == MESSAGE::STATUS::EARLYLEAVE) {
			len = sizeof("무단조퇴");
			stat = new char[len];
			strncpy(stat, "무단조퇴", len);
		}
		break;
	default:
		len = sizeof("Unknown");
		stat = new char[len];
		strcpy(stat, "Unknown");
	}

	return stat;
}

char* Handler::ConvertGoout(packetCommon &packet) {
	int len;
	char *stat;
	
	switch (packet.reason) {
	case MESSAGE::INOUT::OUT:
		len = sizeof("외출나감");
		stat = new char[len];
		strncpy(stat, "외출나감", len); break;
	case MESSAGE::INOUT::IN:
		len = sizeof("외출복귀");
		stat = new char[len];
		strncpy(stat, "외출복귀", len); break;
	default:
		len = sizeof("Unknown");
		stat = new char[len];
		strcpy(stat, "Unknown");
	}

	return stat;
}

char* Handler::ConvertEtc(packetCommon &packet) {
	int len;
	char *stat;

	switch (packet.reason) {
	case MESSAGE::ETC::HEALTH:
		len = sizeof("보건실");
		stat = new char[len];
		strncpy(stat, "보건실", len); break;
	case MESSAGE::ETC::ETC:
		len = sizeof("기타");
		stat = new char[len];
		strncpy(stat, "기타", len); break;
	default:
		len = sizeof("Unknown");
		stat = new char[len];
		strcpy(stat, "Unknown");
	}

	return stat;
}

char* Handler::ConvertName(packetCommon &packet) {
	sql::ResultSet	*res;
	char *name = new char[NAMESIZE + 1];
	memset(name, 0, NAMESIZE + 1);
	
	try {
		NameStmt->setString(1, packet.id);
		res = NameStmt->executeQuery();
		res->next();
		strncpy(name, res->getString("Name").c_str(), NAMESIZE);
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ConvertName(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;

		delete[] name;
		name = new char[sizeof("Unknown")];
		strcpy(name, "Unknown");
	}
	
	return name;
}

//char* Handler::ConvertWeek(packetCommon& packet) {
//	int len = 0;
//	char *week;
//
//	printf("%d\n", packet.week);
//	switch (packet.week) {
//	case MESSAGE::WEEK::MON:
//		len = sizeof("Mon");
//		week = new char[len];
//		strcpy(week, "Mon"); break;
//	case MESSAGE::WEEK::TUE:
//		len = sizeof("Tue");
//		week = new char[len];
//		strcpy(week, "Tue"); break;
//	case MESSAGE::WEEK::WED:
//		len = sizeof("Wed");
//		week = new char[len];
//		strcpy(week, "Wed"); break;
//	case MESSAGE::WEEK::THU:
//		len = sizeof("Thu");
//		week = new char[len];
//		strcpy(week, "Thu"); break;
//	case MESSAGE::WEEK::FRI:
//		len = sizeof("Fri");
//		week = new char[len];
//		strcpy(week, "Fri"); break;
//	default:
//		len = sizeof("Unknown");
//		week = new char[len];
//		strcpy(week, "Unknown");
//	}
//	
//	return week;	
//}

char* Handler::ConvertPeriod(packetCommon& packet) {
	int len = 0;
	char *period;
	
	switch (packet.period) {
	case MESSAGE::PERIOD::FIRST:
		len = sizeof("First");
		period = new char[len];
		strcpy(period, "First"); break;
	case MESSAGE::PERIOD::SECOND:
		len = sizeof("Second");
		period = new char[len];
		strcpy(period, "Second"); break;
	case MESSAGE::PERIOD::THIRD:
		len = sizeof("Third");
		period = new char[len];
		strcpy(period, "Third"); break;
	case MESSAGE::PERIOD::FOURTH:
		len = sizeof("Fourth");
		period = new char[len];
		strcpy(period, "Fourth"); break;
	case MESSAGE::PERIOD::FIFTH:
		len = sizeof("Fifth");
		period = new char[len];
		strcpy(period, "Fifth"); break;
	case MESSAGE::PERIOD::SIXTH:
		len = sizeof("Sixth");
		period = new char[len];
		strcpy(period, "Sixth"); break;
	case MESSAGE::PERIOD::SEVENTH:
		len = sizeof("Seventh");
		period = new char[len];
		strcpy(period, "Seventh"); break;
	default:
		len = sizeof("Unknown");
		period = new char[len];
		strcpy(period, "Unknown");
	}

	return period;
}

int Handler::ConvertPriority(packetCommon &packet) {
	sql::ResultSet *res;
	sql::Statement *stmt = con->createStatement();
	int priority = PRIORITY::NOPRIORITY;

	char query[64] = {0, };
	char *period = ConvertPeriod(packet);
	char *name = ConvertName(packet);

	sprintf(query, "SELECT %s FROM %s WHERE Name = '%s'", period, DBName, name);

	char *status = NULL;
	try {
		res = stmt->executeQuery(query);
		if (res->next()) {
			std::cout << "before res->getString()" << std::endl;
			status = const_cast<char*>(res->getString(period).c_str());
			std::cout << "after res->getString()" << std::endl;
		}
//		else return priority;
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ConvertPriority(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	if (status == NULL) {
		delete[] name;
		delete[] period;
		delete res;
		return priority;
	}
	if (strcmp(status, "질병결석") == 0) priority = PRIORITY::FIRST;
	else if (strcmp(status, "인정결석") == 0) priority = PRIORITY::FIRST;
	else if (strcmp(status, "질병조퇴") == 0) priority = PRIORITY::SECOND;
	else if (strcmp(status, "인정조퇴") == 0) priority = PRIORITY::SECOND;
	else if (strcmp(status, "외출나감") == 0) priority = PRIORITY::THIRD;
	else if (strcmp(status, "질병지각") == 0) priority = PRIORITY::FOURTH;
	else if (strcmp(status, "인정 지각") == 0) priority = PRIORITY::FOURTH;
	else if (strcmp(status, "출석") == 0) priority = PRIORITY::FIFTH;
	else if (strcmp(status, "카드없음") == 0) priority = PRIORITY::SIXTH;

	delete[] name;
	delete[] period;
	delete res;

	return priority;
}

void Handler::ProcNewConn() {
	char type = 0;
	try { Server->accept(); } 
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcNewConn()" << std::endl;
		e.What();
		std::cerr << std::endl;
		return;
	}
	try { Server->recv(&type, sizeof(type)); }
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcNewConn()" << std::endl;
		e.What();
		std::cerr << std::endl;
		return;
	}

	SOCKET ClientSocket = Server->GetClientFD();
	FD_SET(ClientSocket, &ori_reads);
	if (ClientSocket > maxfd)
		maxfd = ClientSocket;
	
	if (type >= '0') type -= '0';
	if (type  == DEVICETYPE::RASPBERRY) {
		std::cout << "Raspberry Connected!" << std::endl;
		RaspSocket = ClientSocket;
	}
	else
		std::cout << "Connected!" << std::endl;
}

void Handler::ProcDisconn(SOCKET fd) {
	close(fd);
	FD_CLR(fd, &ori_reads);

	if (fd == RaspSocket) {
		RaspSocket = 0;
		std::cout << "Raspberry Disconnected!" << std::endl;
	}
	else std::cout << "Disconnected!" << std::endl;
}

void Handler::ProcCommon(SOCKET fd) {
	packetCommon packet;
	memset(&packet, 0, sizeof(packet));

	try {
		Server->recv(packet.id, STUDENTIDSIZE, fd);
		Server->recv(&packet.status, sizeof(packet.status), fd);
		Server->recv(&packet.reason, sizeof(packet.reason), fd); 
		Server->recv(&packet.week, sizeof(packet.week), fd);
		Server->recv(&packet.period, sizeof(packet.period), fd);
	} catch(TcpServerSock::Exception e) { 
		e.What();
		close(fd);
		FD_CLR(fd, &ori_reads);

		return;
	};

	// Replace \n -> 0
	packet.id[STUDENTIDSIZE] = 0;
	// ascii to integer
	if (packet.status >= '0')
		packet.status -= '0';
	if (packet.reason >= '0')
		packet.reason -= '0';
	if (packet.week >= '0')
		packet.week -= '0';
	if (packet.period >= '0')
		packet.period -= '0';

	char *name = ConvertName(packet);
	char *stat = ConvertStatus(packet);
	char *period = ConvertPeriod(packet);
	UpdateTime();

	printf("\nID: %s\n", packet.id);
	printf("Status: %d\n", packet.status);
	printf("Reason: %d\n", packet.reason);
	printf("Week: %d\n", packet.week);
	printf("Period: %d\n", packet.period);
	printf("Name: %s\n", name);
	printf("%s\n\n", stat);

	if (CurPeriod != packet.period) {
		ClearEtc();
		CurPeriod = packet.period;
	}

	ProcRaspberry(packet);

 	try {			
 		EventStmt->setString(1, packet.id);
 		EventStmt->setString(2, stat);
		EventStmt->setString(3, period);
 		EventStmt->setString(4, EventTime);
 		EventStmt->execute();
 	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
 		std::cerr << "Error occured in ProcCommon(), SQL error" << std::endl;
 		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
 	}

	delete[] period;
	delete[] stat;
	delete[] name;
}

void Handler::ProcRaspberry(packetCommon &packet) {
	int myPriority = 0;

	switch (packet.status) {
	case MESSAGE::STATUS::ABSENCE:
		myPriority = PRIORITY::FIRST; break;
	case MESSAGE::STATUS::EARLYLEAVE:
		myPriority = PRIORITY::SECOND; break;
	case MESSAGE::STATUS::GOOUT:
		myPriority = PRIORITY::THIRD; break;
	case MESSAGE::STATUS::LATENESS:
		myPriority = PRIORITY::FOURTH; break;
	case MESSAGE::STATUS::ATTEND:
		myPriority = PRIORITY::FIFTH; break;
	case MESSAGE::STATUS::NOCARD:
		myPriority = PRIORITY::SIXTH; break;
	default:
		myPriority = PRIORITY::NOPRIORITY;
	}

	int priority = ConvertPriority(packet);
	if (myPriority == PRIORITY::NOPRIORITY) {
		sql::Statement *stmt = con->createStatement();
		char *name = ConvertName(packet);
		char *stat = ConvertStatus(packet);
		char query[256] = {0, };
		sprintf(query, "INSERT INTO EtcList(Number, Name, Status) VALUES(%c%c, '%s', '%s') ON DUPLICATE KEY UPDATE Status='%s'", packet.id[2], packet.id[3], name, stat, stat);

		try { stmt->execute(query); } 
		catch (sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcRaspberry(), SQL error" << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}

		delete[] stat;
		delete[] name;
	}
	else if (priority == 0 || myPriority <= priority) {
		MakeAttend();

		char *name = ConvertName(packet);
		char *stat = ConvertStatus(packet);
		char *period = ConvertPeriod(packet);
		
		RunAttendSQL(packet, name, stat, period);	

		delete[] period;
		delete[] stat;
		delete[] name;
	}
}

void Handler::RunAttendSQL(packetCommon& packet, char* name, char *stat, char* period) {
	sql::Statement *stmt;
	char query[256] = {0, };
	
//	sprintf(query, "INSERT INTO %s(Number, Name, %s) VALUES(%c%c, '%s', '%s') ON DUPLICATE KEY UPDATE %s='%s'", DBName, period, packet.id[2], packet.id[3], name, stat, period, stat);
	sprintf(query, "UPDATE %s SET %s='%s' WHERE Name = '%s'", DBName, period, stat, name);
	try {
		stmt = con->createStatement();
		stmt->execute(query);
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in RunAttendSQL(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete stmt;	
}

void Handler::ProcLogin(SOCKET fd) {
	packetLogin packet;
	memset(&packet, 0, sizeof(packet));
	
	try {
		int i = 0;
		while (i < sizeof(packet.id)) {
			Server->recv(&packet.id[i], sizeof(packet.id[i]), fd);
			if (packet.id[i] == '\n') {
				packet.id[i] = 0;
				break;
			}
			i++;
		}
		i = 0;
		while(i < sizeof(packet.password)) {
			Server->recv(&packet.password[i], sizeof(packet.password[i]), fd);
			if (packet.password[i] == '\n') {
				packet.password[i] = 0;
				break;
			}
			i++;
		}
	} catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcLogin(), receive failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);
		return;
	}

	if (CheckLogin(packet)) {
		char success = 1;
		char end[2] = "\r";

		try {
			Server->send(&success, sizeof(success), fd);
			Server->send(end, sizeof(end),fd );
		} catch (TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcLogin(), send name failed" << std::endl;
			e.What();
			std::cerr << std::endl;

			close(fd);
			FD_CLR(fd, &ori_reads);
		}

		std::cout << "Login Success!" << std::endl;
		std::cout << "ID: " << packet.id << std::endl;
	}
	else {
		char fail = 0;
		char end[2] = "\r";

		try { 
			Server->send(&fail, sizeof(fail), fd); 
			Server->send(end, sizeof(end), fd);
		}
		catch (TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcLogin(), send fail message failed" << std::endl;
			e.What();
			std::cerr << std::endl;

			close(fd);
			FD_CLR(fd, &ori_reads);
		}

		std::cout << "Login Failed" << std::endl;
		std::cout << "ID: " << packet.id << std::endl << std::endl;
	}
}

bool Handler::CheckLogin(packetLogin &packet) {
	sql::ResultSet *res;
	bool result = false;
	LoginStmt->setString(1, packet.id);
	try {
		res = LoginStmt->executeQuery();
		res->next();
	
		if (res->getString("ID").compare(packet.id) == 0 &&
			res->getString("Password").compare(packet.password) == 0)
		{
			result = true;
		}
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in CheckLogin(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete res;
	return result;
}	

void Handler::ProcForce(SOCKET fd) {
	packetForce packet;
	memset(&packet, 0, sizeof(packet));

	try {
		int i = 0;
		while (i < sizeof(packet.id)) {
			Server->recv(&packet.id[i], sizeof(packet.id[i]), fd);
			if (packet.id[i] == '\n') {
				packet.id[i] = 0;
				break;
			}
			i++;
		}
		Server->recv(&packet.status, sizeof(packet.status), fd);
		Server->recv(&packet.reason, sizeof(packet.reason), fd);
		i = 0;
		while(i < sizeof(packet.date)) {
			Server->recv(&packet.date[i], sizeof(packet.date[i]), fd);
			if (packet.date[i] == '\n') {
				packet.date[i] = 0;
				break;
			}
			i++;
		}
		Server->recv(&packet.period, sizeof(packet.period), fd);
	} catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcForce(), receive packetForce error" << std::endl;
		e.What();
		std::cerr << std::endl;
	}

	if (packet.status >= '0') packet.status -= '0';
	if (packet.reason >= '0') packet.reason -= '0';
	if (packet.period >= '0') packet.period -= '0';

	printf("\nID: %s\n", packet.id);
	printf("Status: %d\n", packet.status);
	printf("Reason: %d\n", packet.reason);
	printf("Date: %s\n", packet.date);
	printf("Period: %d\n", packet.period);

	// Convert packetForce to packetCommon
	packetCommon common;
	memset(&common, 0, sizeof(common));
	strcpy(common.id, packet.id);
	common.status = packet.status;
	common.reason = packet.reason;
	common.week = 0;
	common.period = packet.period;

	char *name = ConvertName(common);
	char *status = ConvertStatus(common);
	char *period = ConvertPeriod(common);
	char db_name[16] = {0, };
	sprintf(db_name, "%sAttendance", packet.date);

	sql::Statement *stmt = con->createStatement();
	char query[128] = {0, };
	sprintf(query, "UPDATE %s SET %s='%s' WHERE Name = '%s'", db_name, period, status, name);
	try { stmt->execute(query); }
	catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcForce(), sql error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete stmt;
	delete[] period;
	delete[] status;
	delete[] name;
}

void Handler::Run() {
	InitSQL();
	while(Power) {
		reads = ori_reads;
		select(maxfd + 1, &reads, NULL, NULL, NULL);
		
		// If client request new connection
		if (FD_ISSET(ServerSocket, &reads)) {
			ProcNewConn();
			continue;
		}

		// Check changed FD
		for (int fd = ServerSocket + 1; fd <= maxfd; fd++) {
			// If the FD is readable
			if (FD_ISSET(fd, &reads)) {
				char type = 0;
				int res = 0;
				
				try {
					if (fd == RaspSocket) {
						try { res = Server->recv(&type, sizeof(type), fd); }
						catch (TcpServerSock::Exception &e) {
							RaspSocket = 0;
							throw e;
						}
					}
					else res = Server->recv(&type, sizeof(type), fd);
				}
				catch (TcpServerSock::Exception &e) {
					std::cerr << std::endl;
					std::cerr << "Error occured in Run(), receive type failed" << std::endl;
					e.What();
					std::cerr << std::endl;

					close(fd);
					FD_CLR(fd, &ori_reads);
					continue;
				}

				// If client request close connection
				if (res == CLOSE)
					ProcDisconn(fd);
				// If client send message
				else {
					if (type >= '0')
						type -= '0';
					printf("Type: %d\n", type);
					if (type == DATATYPE::COMMON)
						ProcCommon(fd);
					else if (type == DATATYPE::LOGIN)
						ProcLogin(fd);
					else if (type == DATATYPE::FORCE_UPDATE)
						ProcForce(fd);
					else 
						ProcDisconn(fd);
				}
			}
		}
	}
}

void Handler::Stop() { Power = false; }
void Handler::Start() { Power = true; }

