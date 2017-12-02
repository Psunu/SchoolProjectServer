#include "Handler.h"

Handler::Handler() : Power(true) {
	// Socket Setting
	Server = new TcpServerSock(8000);
	ServerSocket = Server->GetServerFD();

	// Select setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// MySQL connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("MooInParkingLot");
}

Handler::~Handler() {
	delete LoginStmt;
	delete GetFeeStmt;
	delete AddFeeStmt;
	delete DelCarStmt;
	delete AddCarStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, ID text, Status text, Date text, Resident text, Fee int)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text)");
	InitStmt->execute("DROP TABLE IF EXISTS CarList");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS CarList(ID text, Date text, Resident text, Fee int)");
	
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(ID, Status, Date, Resident, Fee) VALUES(?, ?, ?, ?, ?)");
	AddCarStmt = con->prepareStatement("INSERT INTO CarList(ID, Date, Resident, Fee) VALUES(?, ?, ?, ?)");
	DelCarStmt = con->prepareStatement("DELETE FROM CarList WHERE ID = ?");
	GetFeeStmt = con->prepareStatement("SELECT Fee FROM CarList WHERE ID = ?");
	AddFeeStmt = con->prepareStatement("UPDATE CarList SET Fee = Fee + 100 WHERE ID = ?");
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
//		Server->recv(&packet, sizeof(packet), fd); 
		Server->recv(&packet.resident, sizeof(packet.resident), fd);
		Server->recv(&packet.status, sizeof(packet.status), fd);
		Server->recv(packet.id, sizeof(packet.id), fd);
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
	packet.id[CARIDSIZE] = 0;
	// ascii to integer
	packet.resident -= '0';
	packet.status -= '0';

	// Print packet information
	printf("\nResident: %d\n", packet.resident);
	printf("Status: %d\n", packet.status);
	printf("ID: %s\n", packet.id);

	UpdateTime();
	if (packet.resident == PACKET::RESIDENT::YES)
		ProcResident(packet);
	else if (packet.resident == PACKET::RESIDENT::NO)
		ProcCustomer(packet, fd);
}

void Handler::ProcResident(const packet_message &packet) {
	char *stat;
	if (packet.status == PACKET::INOUT::IN) {
		int size = sizeof("In");
		stat = new char[size];
		strcpy(stat, "In");

		try {
			AddCarStmt->setString(1, packet.id);
			AddCarStmt->setString(2, EventTime);
			AddCarStmt->setString(3, "O");
			AddCarStmt->setInt(4, 0);
			AddCarStmt->execute();
		} catch (sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcResident(), AddCarStmt SQL error" << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}
	}
	else if (packet.status == PACKET::INOUT::OUT) {
		int size = sizeof("Out");
		stat = new char[size];
		strcpy(stat, "Out");

		try {
			DelCarStmt->setString(1, packet.id);
			DelCarStmt->execute();
		} catch (sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcResident(), DelCarStmt SQL error" << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}
	}

	try {
		EventStmt->setString(1, packet.id);
		EventStmt->setString(2, stat);
		EventStmt->setString(3, EventTime);
		EventStmt->setString(4, "O");
		EventStmt->setInt(5, 0);
		EventStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcResident(), EventStmt SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete[] stat;
}

void Handler::ProcCustomer(packet_message &packet, SOCKET fd) {
	if (packet.status == PACKET::INOUT::IN)
		ProcIn(packet);
	else if (packet.status == PACKET::INOUT::OUT) {
		packet_fee packetFee;
		memset(&packetFee, 0, sizeof(packetFee));
		
		int fee = ProcOut(packet);
		strcpy(packetFee.id, packet.id);
		packetFee.fee = fee;

		try { Server->send(&packetFee, sizeof(packetFee), fd); }
		catch(TcpServerSock::Exception &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcCustomer(), send failed" << std::endl;
			e.What();
			std::cerr << std::endl;
		}

		std::cout << std::endl;
		std::cout << "Car exit" << std::endl;
		std::cout << "ID: " << packetFee.id << std::endl;
		std::cout << "Fee: " << packetFee.fee << std::endl;
		std::cout << std::endl;
	}
}	

void Handler::ProcIn(packet_message &packet) {
	char *id = new char[CARIDSIZE + 1];
	strcpy(id, packet.id);

	std::thread thread(&Handler::ThreadRun, this, id);
	pthread_t thread_id = thread.native_handle();
	thread.detach();
	CarList.insert(std::pair<std::string, pthread_t>(packet.id, thread_id));

	try {
		EventStmt->setString(1, packet.id);
		EventStmt->setString(2, "In");
		EventStmt->setString(3, EventTime);
		EventStmt->setString(4, "X");
		EventStmt->setInt(5, DEFAULT_FEE);
		
		AddCarStmt->setString(1, packet.id);
		AddCarStmt->setString(2, EventTime);
		AddCarStmt->setString(3, "X");
		AddCarStmt->setInt(4, DEFAULT_FEE);
	
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
	std::map<std::string, pthread_t>::iterator it;
	it = CarList.find(packet.id);

	int ret = 0;
	int status = 0;
	pthread_t thread_id = it->second;
	ret = pthread_cancel(thread_id);
	pthread_join(thread_id, (void **)&status);
	CarList.erase(it);

	int fee = 0;		
	try {
		GetFeeStmt->setString(1, packet.id);
		sql::ResultSet *result = GetFeeStmt->executeQuery();
		result->next();
		fee = result->getInt("Fee");

		EventStmt->setString(1, packet.id);
		EventStmt->setString(2, "Out");
		EventStmt->setString(3, EventTime);
		EventStmt->setString(4, "X");
		EventStmt->setInt(5, fee);
	
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

void Handler::ThreadRun(char* id) {
	char carID[CARIDSIZE + 1] = {0, };
	strcpy(carID, id);
	delete[] id;

	std::cout << std::endl;
	std::cout << "Thread Started!" << std::endl;
	std::cout << "ID : " << carID << std::endl;
	std::cout << std::endl;

	while(true) {
		std::this_thread::sleep_for(std::chrono::seconds(60));
		thread_mutex.lock();
		try {
			AddFeeStmt->setString(1, carID);
			AddFeeStmt->execute();
		} catch(sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ThreadRun(), SQL error" << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
			break;
		}
		thread_mutex.unlock();
	}
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
		std::cout << std::endl;
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
		std::cout << std::endl;
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
	//				type = static_cast<char>(atoi(&type));
					type -= '0';
					printf("type: %d, size: %d, 0: %d\n", type, sizeof(type), '0');
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
