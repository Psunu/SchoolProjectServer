#include "Handler.h"

Handler::Handler() : Power(true) {
	// Socket Setting
	Server = new TcpServerSock(8001);
	ServerSocket = Server->GetServerFD();

	// Select setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// MySQL connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("Elevator");
}

Handler::~Handler() {
	delete LoginStmt;
	delete FloorStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, Event text, Date text)");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS UserInfo(ID text, Password text)");
	InitStmt->execute("DROP TABLE IF EXISTS CurrentFloor");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS CurrentFloor(Floor int, Date text)");
	InitStmt->execute("INSERT INTO CurrentFloor(Floor, Date) values(0, 'Unknown')");
	
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(Event, Date) VALUES(?, ?)");
	FloorStmt = con->prepareStatement("UPDATE CurrentFloor SET Floor = ?, Date = ?");
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

void Handler::ProcFloor(SOCKET fd) {
	packetFloor packet = 0; 

	try { Server->recv(&packet, sizeof(packet), fd); }
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcFloor(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);

		return;
	}

	// Print packet information
	printf("\nFloor: %d\n\n", packet);

	char event[128] = {0, };
	UpdateTime();

	if (packet > 0 && packet < 6) {
		// Write event details
		sprintf(event, "Current floor : %d", packet);
		try {
			EventStmt->setString(1, event);
			EventStmt->setString(2, EventTime);
			FloorStmt->setInt(1, static_cast<int>(packet));
			FloorStmt->setString(2, EventTime);
			
			EventStmt->execute();
			FloorStmt->execute();
		} catch (sql::SQLException &e) {
			std::cerr << std::endl;
			std::cerr << "Error occured in ProcFloor(), SQL error" << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}
	}
	else {
		std::cout << std::endl;
		std::cout << "Unknown floor" << std::endl;
		std::cout << std::endl;
	}
}

void Handler::ProcButton(SOCKET fd) {
	packetButton packet;
	memset(&packet, 0, sizeof(packet));

	try { 
		Server->recv(&packet.floor, sizeof(packet.floor), fd);
		Server->recv(&packet.button, sizeof(packet.button), fd);
	}
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcButton(), receiving failed" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);

		return;
	}

	if (packet.floor >= '0') packet.floor -= '0';
	if (packet.button >= '0') packet.button -= '0';

	// Print packet information
	printf("\nFloor: %d\n", packet.floor);
	printf("Button: %d\n", packet.button);

	char event[128] = {0, };
	UpdateTime();

	if (packet.button == PACKET::BUTTON::DOWN) {
		sprintf(event, "Floor : %d, Down button pushed", packet.floor);
	} 
	else if (packet.button == PACKET::BUTTON::UP) {
		sprintf(event, "Floor : %d, Up button pushed", packet.floor);
	} 
	else {
		std::cout << std::endl;
		std::cout << "Unknown buttonn" << std::endl;
		std::cout << std::endl;
		return;
	}

	try {
		EventStmt->setString(1, event);
		EventStmt->setString(2, EventTime);
		EventStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcButton(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
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

bool Handler::CheckLogin(packetLogin &packet) {
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

					if (type == PACKET::TYPE::FLOOR)
						ProcFloor(fd);
					else if (type == PACKET::TYPE::BUTTON)
						ProcButton(fd);
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
