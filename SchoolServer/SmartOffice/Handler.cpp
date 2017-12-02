#include "Handler.h"

Handler::Handler() : Power(true), RaspSocket(0) {
	// Socket Setting
	Server = new TcpServerSock;
	ServerSocket = Server->GetServerFD();

	// Select setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// MySQL connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("SmartOffice");
}

Handler::~Handler() {
	delete LoginStmt;
	delete CurrentStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, Name text, Status text, Date text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text, Name text)");
 	InitStmt->execute("DROP TABLE IF EXISTS CurrentStatus"); 	
 	InitStmt->execute("CREATE TABLE IF NOT EXISTS CurrentStatus(Name text, Status text, Date text)");
	// Initialize Table value
	InitStmt->execute("INSERT INTO CurrentStatus(Name, Status) VALUES('이두희', 'Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Name, Status) VALUES('윤상종', 'Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Name, Status) VALUES('김진섭','Unknown')");
 	InitStmt->execute("INSERT INTO CurrentStatus(Name, Status) VALUES('최윤희', 'Unknown')");
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(Name, Status, Date) VALUES(?, ?, ?)");
	CurrentStmt = con->prepareStatement("UPDATE CurrentStatus SET Status = ?, Date = ? WHERE Name = ?");
	LoginStmt = con->prepareStatement("SELECT ID, Password, Name FROM UserInfo WHERE ID=?");

	delete InitStmt;
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
	case MESSAGE::STATUS::IN:
		len = sizeof("재실중");
		stat = new char[len];
		strncpy(stat, "재실중", len); break;
	case MESSAGE::STATUS::LESSON:
		len = sizeof("수업중");
		stat = new char[len];
		strncpy(stat, "수업중", len); break;
	case MESSAGE::STATUS::OUTING:
		len = sizeof("외출중");
		stat = new char[len];
		strncpy(stat, "외출중", len); break;
	case MESSAGE::STATUS::BUSINESSTRIP:
		len = sizeof("출장중");
		stat = new char[len];
		strncpy(stat, "출장중", len); break;
	case MESSAGE::STATUS::OFFWORK:
		len = sizeof("퇴근");
		stat = new char[len];
		strncpy(stat, "퇴근", len); break;
	default:
		len = sizeof("Unknown");
		stat = new char[len];
		strcpy(stat, "Unknown"); break;
	}

	return stat;
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

		close(Server->GetClientFD());
		return;
	}
	

	SOCKET ClientSocket = Server->GetClientFD();
	FD_SET(ClientSocket, &ori_reads);
	if (ClientSocket > maxfd)
		maxfd = ClientSocket;

	if (type - '0' == DEVICETYPE::RASPBERRY) {
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
	else
		std::cout << "Disconnected!" << std::endl;
}

void Handler::ForwardToRasp(packetCommon &packet) {
	try {
		Server->send(packet.name, NAMESIZE, RaspSocket);
		Server->send(&packet.status, sizeof(packet.status), RaspSocket);
	}
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ForwardToRasp()" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(RaspSocket);
		FD_CLR(RaspSocket, &ori_reads);
		RaspSocket = 0;

		return;
	}
	std::cout << "Packet has forwarded!" << std::endl;
}

void Handler::ProcCommon(SOCKET fd) {
	packetCommon packet; 
	memset(&packet, 0, sizeof(packet));

	try { 
		if (fd == RaspSocket) {
			try {
				Server->recv(packet.name, NAMESIZE, fd);
				Server->recv(&packet.status, sizeof(packet.status), fd);
			}
			catch (TcpServerSock::Exception &e) {
				RaspSocket = 0;
				throw e;
			}
		}
		else 
			Server->recv(&packet, sizeof(packet), fd);
	}
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcCommon(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);

		return;
	}

	// Replace \n -> 0
	packet.name[NAMESIZE] = 0;

	if (packet.status >= '0')
		packet.status -= '0';
	char *stat = ConvertStatus(packet);

	// Print packet information
	printf("\nFD: %d\n", fd);
	printf("Name: %s\n", packet.name);
	printf("Status: %d\n", packet.status);
	printf("%s\n\n", stat);

	// If client is not raspberry
	if (fd != RaspSocket && RaspSocket != 0)
		ForwardToRasp(packet);

	// Process SQL
	UpdateTime();
	try {
		EventStmt->setString(1, packet.name);
		EventStmt->setString(2, stat);
		EventStmt->setString(3, EventTime);
		EventStmt->execute();
	
		CurrentStmt->setString(1, stat);
		CurrentStmt->setString(2, EventTime);
		CurrentStmt->setString(3, packet.name);
		CurrentStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcCommon(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete[] stat;
}

void Handler::ProcLogin(SOCKET fd) {
	packetLogin packet;
	memset(&packet, 0, sizeof(packet));	
	
	try {
		int i = 0;
		while(i < sizeof(packet.id)) {
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
		std::cerr << "Error occured in ProcLogin(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);
		return;
	}

	char *name;
	if ((name = CheckLogin(packet)) != NULL) {
		packetLoginSuccess packetSuccess;
		memset(&packetSuccess, 0, sizeof(packetSuccess));

		packetSuccess.flag = 1;
		strncpy(packetSuccess.name, name, NAMESIZE);
		char end[2] = "\r";

		try {
			Server->send(&packetSuccess.flag, sizeof(packetSuccess.flag), fd);
			Server->send(end, sizeof(end), fd);
			Server->send(packetSuccess.name, NAMESIZE, fd);
			Server->send(end, sizeof(end), fd);
		}
		catch (TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcLogin(), sending name failed" << std::endl;
			e.What();
			std::cerr << std::endl;

			close(fd);
			FD_CLR(fd, &ori_reads);
		}

		std::cout << std::endl;
		std::cout << "Login Success!" << std::endl;
		std::cout << "ID: " << packet.id << std::endl;
		std::cout << "Name: " << name << std::endl << std::endl;

		delete[] name;
	}
	else {
		char flag = 0;
		char end[2] = "\r";

		try { 
			Server->send(&flag, sizeof(flag), fd); 
			Server->send(end, sizeof(end), fd);
		}
		catch (TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcLogin(), sending fail message failed" << std::endl;
			e.What();
			std::cerr << std::endl;

			close(fd);
			FD_CLR(fd, &ori_reads);
		}

		std::cout << std::endl;
		std::cout << "Login Failed" << std::endl;
		std::cout << "ID: " << packet.id << std::endl << std::endl;
	}
}

char* Handler::CheckLogin(packetLogin &packet) {
	sql::ResultSet	*res;
	char *name = new char[NAMESIZE + 1];
	memset(name, 0, NAMESIZE + 1);
	
	try { 
		LoginStmt->setString(1, packet.id);
		res = LoginStmt->executeQuery();
		res->next();
	
		if (res->getString("ID").compare(packet.id) == 0 &&
			res->getString("Password").compare(packet.password) == 0)
		{ 
			strncpy(name, res->getString("Name").c_str(), NAMESIZE);
		}
		else {
			delete[] name;
			name = NULL;
		}
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in CheckLogin(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;

		delete[] name;
		name = NULL;
	} 
			
	delete res;
	return name;
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
					else
						res = Server->recv(&type, sizeof(type), fd);
				}
				catch (TcpServerSock::Exception &e) { 
					std::cerr << std::endl;
					std::cerr << "Error occured in Run(), receiving type failed" << std::endl;
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
					if (type == DATATYPE::COMMON)
						ProcCommon(fd);
					else if (type == DATATYPE::LOGIN)
						ProcLogin(fd);
					else
						ProcDisconn(fd);
				}
			}
		}
	}
}

void Handler::Stop() { Power = false; }
void Handler::Start() { Power = true; }
