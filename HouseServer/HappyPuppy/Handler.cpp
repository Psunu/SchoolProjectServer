#include "Handler.h"

Handler::Handler() : Power(true), RaspSocket(0) {
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
	con->setSchema("HappyPuppy");
}

Handler::~Handler() {
	delete AmountStmt;
	delete EventStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	sql::Statement *InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, Event text, Date text)");
 	InitStmt->execute("DROP TABLE IF EXISTS Amount"); 	
 	InitStmt->execute("CREATE TABLE IF NOT EXISTS Amount(Feed1 text, Feed2 text, Feed3 text, Water text, Date text)");
	InitStmt->execute("INSERT INTO Amount(Feed1, Feed2, Feed3, Water) VALUES('Unknown', 'Unknown', 'Unknown', 'Unknown')");
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(Event, Date) VALUES(?, ?)");
	AmountStmt = con->prepareStatement("UPDATE Amount SET Feed1 = ?, Feed2 = ?, Feed3 = ?, Water = ?, Date = ?");

	delete InitStmt;
}

// Update Current Time
void Handler::UpdateTime() {
	memset(EventTime, 0, sizeof(EventTime));
	// Year-Month-Day-Hour-Minute
	sprintf(EventTime, "%d-%02d-%02d %02d:%02d", tm.Get_year(), tm.Get_mon(), tm.Get_day(), tm.Get_hour(), tm.Get_min());
}

char* Handler::ConvertAmount(const char feed_amount) {
	int len = 0;
	char *amount;
	
	switch (feed_amount) {
	case MESSAGE::AMOUNT::LOW:
		len = sizeof("Low");
		amount = new char[len];
		strcpy(amount, "Low"); break;
	case MESSAGE::AMOUNT::MIDDLE:
		len = sizeof("Middle");
		amount = new char[len];
		strcpy(amount, "Middle"); break;
	case MESSAGE::AMOUNT::LOT:
		len = sizeof("Lot");
		amount = new char[len];
		strcpy(amount, "Lot"); break;
	default:
		amount = NULL;
	}

	return amount;
}

char* Handler::ConvertType(const char feed_type) {
	int len = 0;
	char *type;

	switch (feed_type) {
	case MESSAGE::TYPE::FEED1:
		len = sizeof("Feed1");
		type = new char[len];
		strcpy(type, "Feed1"); break;
	case MESSAGE::TYPE::FEED2:
		len = sizeof("Feed2");
		type = new char[len];
		strcpy(type, "Feed2"); break;
	case MESSAGE::TYPE::FEED3:
		len = sizeof("Feed3");
		type = new char[len];
		strcpy(type, "Feed3"); break;
	case MESSAGE::TYPE::WATER:
		len = sizeof("Water");
		type = new char[len];
		strcpy(type, "Water"); break;
	default:
		type = NULL;
	}

	return type;
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

	std::cout << "Connected!" << std::endl;
}

void Handler::ProcDisconn(SOCKET fd) {
	close(fd);
	FD_CLR(fd, &ori_reads);
	std::cout << "Disconnected!" << std::endl;
}

void Handler::ProcShowAmount(SOCKET fd) {
	packetShowAmount packet;
	memset(&packet, 0, sizeof(packet));

	try { 
		Server->recv(&packet.feed1_amount, sizeof(packet.feed1_amount), fd);
		Server->recv(&packet.feed2_amount, sizeof(packet.feed2_amount), fd);
		Server->recv(&packet.feed3_amount, sizeof(packet.feed3_amount), fd);
		Server->recv(&packet.water_amount, sizeof(packet.water_amount), fd);
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

	// if packet is ascii charactor
	if (packet.feed1_amount >= '0') 
		packet.feed1_amount -= '0';
	if (packet.feed2_amount >= '0')
		packet.feed2_amount -= '0';
	if (packet.feed3_amount >= '0')
		packet.feed3_amount -= '0';
	if (packet.water_amount >= '0')
		packet.water_amount -= '0';	

	// Print packet information
	printf("\nShowAmount packet\n");
	printf("Feed1 Amount: %d\n", packet.feed1_amount);
	printf("Feed2 Amount: %d\n", packet.feed2_amount);
	printf("Feed3 Amount: %d\n", packet.feed3_amount);
	printf("Water Amount: %d\n\n", packet.water_amount);

	// Process SQL
	UpdateTime();

	char *feed1_amount = ConvertAmount(packet.feed1_amount);
	char *feed2_amount = ConvertAmount(packet.feed2_amount);
	char *feed3_amount = ConvertAmount(packet.feed3_amount);
	char *water_amount = ConvertAmount(packet.water_amount);

	char event[128] = {0, };
	if (feed1_amount != NULL && feed2_amount != NULL && feed3_amount != NULL && water_amount !=NULL) {
		sprintf(event, "Amount update Feed1 : %s, Feed2 : %s, Feed3 : %s, Water : %s" , feed1_amount, feed2_amount, feed3_amount, water_amount);
	}
	else
		return;

	try {
		EventStmt->setString(1, event);
		EventStmt->setString(2, EventTime);
	
		AmountStmt->setString(1, feed1_amount);
		AmountStmt->setString(2, feed2_amount);
		AmountStmt->setString(3, feed3_amount);
		AmountStmt->setString(4, water_amount);
		AmountStmt->setString(5, EventTime);
	
		EventStmt->execute();
		AmountStmt->execute();
	} catch(sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcShowAmount, SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	if (feed1_amount != NULL)
		delete[] feed1_amount;
	if (feed2_amount != NULL)
		delete[] feed2_amount;
	if (feed3_amount != NULL)
		delete[] feed3_amount;
	if (water_amount != NULL)
		delete[] water_amount;
}

void Handler::ProcFeed(SOCKET fd) {
	packetFeed packet;
	memset(&packet, 0, sizeof(packet));

	try {
		Server->recv(&packet.feed, sizeof(packet.feed), fd);
		Server->recv(&packet.feed_amount, sizeof(packet.feed_amount), fd);
	} catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcFeed(), receive error" << std::endl;
		e.What();
		std::cerr << std::endl;

		close(fd);
		FD_CLR(fd, &ori_reads);
		
		return;
	}

	if (packet.feed >= '0') 
		packet.feed -= '0';
	if (packet.feed_amount >= '0')
		packet.feed_amount -= '0';
	
	printf("\nFeed packet\n");
	printf("Feed: %d", packet.feed);
	printf("Feed Amount: %d\n", packet.feed_amount);

	UpdateTime();
	char *feed_name = ConvertType(packet.feed);
	char *feed_amount = ConvertAmount(packet.feed_amount);

	char event[128] = {0, };
	sprintf(event, "Feeding Feed Type : %s, Amount : %s", feed_name, feed_amount);
	
	try {
		EventStmt->setString(1, event);
		EventStmt->setString(2, EventTime);
		EventStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcFeed(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
	
	delete[] feed_name;
	delete[] feed_amount;
}

void Handler::ProcShowPicture() {
	printf("\nShowPicture packet\n");

	char event[] = "Show Picture";
	try {
		EventStmt->setString(1, event);
		EventStmt->setString(2, EventTime);
		EventStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcShowPicture(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
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
					// If type is ascii charactor
					if (type >= '0')
						type -= '0';

					if (type == DATATYPE::SHOW_AMOUNT)
						ProcShowAmount(fd);
					else if (type == DATATYPE::SHOW_PICTURE)
						ProcShowPicture();
					else if (type == DATATYPE::FEED)
						ProcFeed(fd);
					else
						ProcDisconn(fd);
				}
			}
		}
	}
}

void Handler::Stop() { Power = false; }
void Handler::Start() { Power = true; }
