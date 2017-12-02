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
	con->setSchema("ParkingTower");
}

Handler::~Handler() {
	delete LoginStmt;
	delete DelCarStmt;
	delete AddCarStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, ID text, Location text, Status text, Fee int, Date text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text)");
	InitStmt->execute("DROP TABLE IF EXISTS CarList");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS CarList(ID text, Location text, Date text)");
	
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(ID, Location, Status, Fee, Date) VALUES(?, ?, ?, ?, ?)");
	AddCarStmt = con->prepareStatement("INSERT INTO CarList(ID, Location, Date) VALUES(?, ?, ?)");
	DelCarStmt = con->prepareStatement("DELETE FROM CarList WHERE ID = ?");
	LoginStmt = con->prepareStatement("SELECT ID, Password FROM UserInfo WHERE ID=?");

	delete InitStmt;
}

// Update Current Time
void Handler::UpdateTime() {
	memset(EventTime, 0, sizeof(EventTime));
	// Year-Month-Day-Hour-Minute
	sprintf(EventTime, "%d-%02d-%02d %02d:%02d", tm.Get_year(), tm.Get_mon(), tm.Get_day(), tm.Get_hour(), tm.Get_min());
}

void Handler::ProcNewConn() {
	try { Server->accept(); }
	catch (TcpServerSock::Exception &e) {
		std::cerr << "Error occured in ProcNewConn()" << std::endl;
		e.What();
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
//		Server->recv(&packet, sizeof(packet), fd); 
		Server->recv(&packet.status, sizeof(packet.status), fd);
		Server->recv(packet.id, sizeof(packet.id), fd);
		Server->recv(packet.location, sizeof(packet.location), fd);
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

	// ascii to integer
	packet.status -= '0';
	// Print packet information
	printf("\nStatus: %d\n", packet.status);
	printf("\nID: %s\n", packet.id);
	printf("\nLocation: %s\n", packet.location);

	UpdateTime();
	if (packet.status == PACKET::INOUT::IN) 
		ProcIn(packet);
	else {
		packet_fee fee;
		memset(&fee, 0, sizeof(fee));
		strcpy(fee.id, packet.id);
		fee.fee = ProcOut(packet);

		try {
//			Server->send(&fee, sizeof(fee), fd);
			Server->send(fee.id, fd);
			Server->send(&fee.fee, fd);
		} catch (TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcMessage(), sending failed" << std::endl;
			e.What();
			std::cerr << std::endl;
			
			close(fd);
			FD_CLR(fd, &ori_reads);
	
			return;
		}
	}
}

void Handler::ProcIn(packet_message &packet) {
	CarList.insert(std::pair<std::string, int>(packet.id, time(NULL) / 60));

	try {
		EventStmt->setString(1, packet.id);
		EventStmt->setString(2, packet.location);
		EventStmt->setString(3, "In");
		EventStmt->setInt(4, DEFAULT_FEE);
		EventStmt->setString(5, EventTime);
		
		AddCarStmt->setString(1, packet.id);
		AddCarStmt->setString(2, packet.location);
		AddCarStmt->setString(3, EventTime);
	
		EventStmt->execute();
		AddCarStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcIn(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
}

int Handler::ProcOut(packet_message &packet) {
	int fee = DEFAULT_FEE + CalcFee(packet);
			
	try {
		EventStmt->setString(1, packet.id);
		EventStmt->setString(2, packet.location);
		EventStmt->setString(3, "Out");
		EventStmt->setInt(4, fee);
		EventStmt->setString(5, EventTime);
	
		DelCarStmt->setString(1, packet.id);
		
		EventStmt->execute();
		DelCarStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcOut(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;	
	}

	return fee;	
}

int Handler::CalcFee(packet_message& packet) {
	std::map<std::string, int>::iterator it;
	it = CarList.find(packet.id);

	int inTime = it->second;
	int outTime = time(NULL) / 60;
	CarList.erase(it);

	return (outTime - inTime) / FEE_TIME * FEE_MONEY;
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

	LoginStmt->setString(1, packet.id);
	try { 
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
