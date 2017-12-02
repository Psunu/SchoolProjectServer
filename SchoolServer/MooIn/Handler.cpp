#include "Handler.h"

Handler::Handler() : Power(true) {
	// Socket Setting
	Server = new TcpServerSock(8002);
	ServerSocket = Server->GetServerFD();

	// Select setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// MySQL connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("MooIn");
}

Handler::~Handler() {
	delete LoginStmt;
	delete CommFlexStmt;
	delete CommPushupStmt;
	delete CommRunStmt;
	delete CircuitFlexStmt;
	delete CircuitPushupStmt;
	delete CircuitRunStmt;
	delete ControlFlexStmt;
	delete ControlPushupStmt;
	delete ControlRunStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Control_Run50M(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Control_PushUp(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Control_Flexibility(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Circuit_Run50M(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Circuit_PushUp(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Circuit_Flexibility(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Communication_Run50M(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Communication_PushUp(StudentID text, Name text, Record text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS Communication_Flexibility(StudentID text, Name text, Record text)");

	// PreparedStatement Setting	
	LoginStmt = con->prepareStatement("SELECT ID, Password FROM UserInfo WHERE ID=?");
	ControlFlexStmt = con->prepareStatement("INSERT INTO Control_Flexibility(StudentID, Name, Record) values(?, ?, ?)");
	ControlPushupStmt = con->prepareStatement("INSERT INTO Control_PushUp(StudentID, Name, Record) values(?, ?, ?)");
	CircuitRunStmt = con->prepareStatement("INSERT INTO Circuit_Run50M(StudentID, Name, Record) values(?, ?, ?)");
	CircuitFlexStmt = con->prepareStatement("INSERT INTO Circuit_Flexibility(StudentID, Name, Record) values(?, ?, ?)");
	CircuitPushupStmt = con->prepareStatement("INSERT INTO Circuit_PushUp(StudentID, Name, Record) values(?, ?, ?)");
	CommRunStmt = con->prepareStatement("INSERT INTO Communication_Run50M(StudentID, Name, Record) values(?, ?, ?)");
	CommFlexStmt = con->prepareStatement("INSERT INTO Communication_Flexibility(StudentID, Name, Record) values(?, ?, ?)");
	CommPushupStmt = con->prepareStatement("INSERT INTO Communication_PushUp(StudentID, Name, Record) values(?, ?, ?)");
	CommRunStmt = con->prepareStatement("INSERT INTO Communication_Run50M(StudentID, Name, Record) values(?, ?, ?)");

	delete InitStmt;
}

// Update Current Time
void Handler::UpdateTime() {
	memset(EventTime, 0, sizeof(EventTime));
	// Year-Month-Day-Hour-Minute
	sprintf(EventTime, "%d-%02d-%02d %02d:%02d", tm.Get_year(), tm.Get_mon(), tm.Get_day(), tm.Get_hour(), tm.Get_min());
}

char* Handler::ConvertDept(packet_message &packet) {
	char* dept;
	int len;

	switch(packet.id[0] - '0') {
	case DEPARTMENT::CONTROL:
		len = sizeof("제어");
		dept = new char[len];
		strncpy(dept, "제어", len);
		break;
	case DEPARTMENT::CIRCUIT:
		len = sizeof("회로");
		dept = new char[len];
		strncpy(dept, "회로", len);
		break;
	case DEPARTMENT::COMMUNICATION:
		len = sizeof("통신");
		dept = new char[len];
		strncpy(dept, "통신", len);
		break;
	default:
		dept = NULL;
	}

	return dept;
}

void Handler::ProcNewConn() {
	try { Server->accept(); }
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

	std::cout << "Connectd!" << std::endl;
}

void Handler::ProcDisconn(SOCKET fd) {
	close(fd);
	FD_CLR(fd, &ori_reads);

	std::cout << "Disconnected!" << std::endl;
}

void Handler::ProcMessage(SOCKET fd) {
	packet_message packet; 
	memset(&packet, 0, sizeof(packet));

	try { 
		Server->recv(&packet.event, sizeof(packet.event), fd);
		Server->recv(packet.id, STIDSIZE, fd);
		int i = 0;
		do { Server->recv(&packet.name[i], sizeof(char), fd); }
		while (packet.name[i++] != '!');
		packet.name[i - 1] = 0;
		Server->recv(packet.record, RECSIZE, fd);
	}
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcMessage(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);

		return;
	}

	// Replace \n -> 0
	packet.id[STIDSIZE] = 0;

	// packet.name[NAMESIZE] = 0;
	if (packet.event >= '0')
		packet.event -= '0';

	// Print packet information
	printf("\nEvent: %d\n", packet.event);
	printf("ID: %s\n", packet.id);
	printf("Name: %s\n", packet.name);
	printf("Record: %s\n\n", packet.record);

	switch(packet.event) {
	case PACKET::EVENT::RUN50M:
		ProcRun(packet);
		break;
	case PACKET::EVENT::PUSHUP:
		ProcPushup(packet);
		break;
	case PACKET::EVENT::FLEX:
		ProcFlex(packet);
		break;
	}
}

void Handler::ProcRun(packet_message &packet) {
	char *dept = ConvertDept(packet);

	try {
		switch(packet.id[0] - '0') {
		case DEPARTMENT::CONTROL:
			ControlRunStmt->setString(1, packet.id);
			ControlRunStmt->setString(2, packet.name);
			ControlRunStmt->setString(3, packet.record);
			ControlRunStmt->execute(); break;
		case DEPARTMENT::CIRCUIT:
			CircuitRunStmt->setString(1, packet.id);
			CircuitRunStmt->setString(2, packet.name);
			CircuitRunStmt->setString(3, packet.record);
			CircuitRunStmt->execute(); break;
		case DEPARTMENT::COMMUNICATION:
			CommRunStmt->setString(1, packet.id);
			CommRunStmt->setString(2, packet.name);
			CommRunStmt->setString(3, packet.record);
			CommRunStmt->execute(); break;
		default:
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcRun(), Department not found" << std::endl;
			std::cerr << std::endl;			
		}
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcRun(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	if (dept != NULL)
		delete[] dept;
}

void Handler::ProcPushup(packet_message &packet) {
	char *dept = ConvertDept(packet);

	try {
		switch(packet.id[0] - '0') {
		case DEPARTMENT::CONTROL:
			ControlPushupStmt->setString(1, packet.id);
			ControlPushupStmt->setString(2, packet.name);
			ControlPushupStmt->setString(3, packet.record);
			ControlPushupStmt->execute(); break;
		case DEPARTMENT::CIRCUIT:
			CircuitPushupStmt->setString(1, packet.id);
			CircuitPushupStmt->setString(2, packet.name);
			CircuitPushupStmt->setString(3, packet.record);
			CircuitPushupStmt->execute(); break;
		case DEPARTMENT::COMMUNICATION:
			CommPushupStmt->setString(1, packet.id);
			CommPushupStmt->setString(2, packet.name);
			CommPushupStmt->setString(3, packet.record);
			CommPushupStmt->execute(); break;
		default:
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcPushup(), Department not found" << std::endl;
			std::cerr << std::endl;
		}

	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcPushup(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	if (dept != NULL)
		delete[] dept;
}

void Handler::ProcFlex(packet_message &packet) {
	char *dept = ConvertDept(packet);

	try {
		switch(packet.id[0] - '0') {
		case DEPARTMENT::CONTROL:
			ControlFlexStmt->setString(1, packet.id);
			ControlFlexStmt->setString(2, packet.name);
			ControlFlexStmt->setString(3, packet.record);
			ControlFlexStmt->execute(); break;
		case DEPARTMENT::CIRCUIT:
			CircuitFlexStmt->setString(1, packet.id);
			CircuitFlexStmt->setString(2, packet.name);
			CircuitFlexStmt->setString(3, packet.record);
			CircuitFlexStmt->execute(); break;
		case DEPARTMENT::COMMUNICATION:
			CommFlexStmt->setString(1, packet.id);
			CommFlexStmt->setString(2, packet.name);
			CommFlexStmt->setString(3, packet.record);
			CommFlexStmt->execute(); break;
		default:
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcFlex(), Department not found" << std::endl;
			std::cerr << std::endl;
		}

	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcFlex(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	if (dept != NULL)
		delete[] dept;
}

void Handler::ProcLogin(SOCKET fd) {
	packet_login packet;
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
		while (i < sizeof(packet.pw)) {
			Server->recv(&packet.pw[i], sizeof(packet.pw[i]), fd);
			if (packet.pw[i] == '\n') {
				packet.pw[i] = 0;
				break;
			}
			i++;
		}

	//	Server->recv(&packet, sizeof(packet), fd);
	} catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcLogin(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);
		return;
	}

	if (CheckLogin(packet)) {
		char flag = 1;
		char end[2] = "\r";

		try { 
			Server->send(&flag, sizeof(flag), fd);
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
		std::cout << "PW: " << packet.pw << std::endl;
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

bool Handler::CheckLogin(packet_login &packet) {
	sql::ResultSet	*res;
	bool flag = false;

	try { 
		LoginStmt->setString(1, packet.id);
		res = LoginStmt->executeQuery();
		res->next();
	
		if (res->getString("ID").compare(packet.id) == 0 &&
			res->getString("Password").compare(packet.pw) == 0)

		{ 
			flag = true;
		}
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in CheckLogin(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	} 

	delete res;
	return flag;
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

				try { res = Server->recv(&type, sizeof(type), fd); }
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
					// type = static_cast<char>(atoi(&type));
					if (type >= '0')
						type -= '0';
					if (type == PACKET::TYPE::MESSAGE)
						ProcMessage(fd);
					else if (type == PACKET::TYPE::LOGIN)
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
