#include "Handler.h"

Handler::Handler() : Power(true) {
	// Socket Setting
	Server = new TcpServerSock;
	ServerSocket = Server->GetServerFD();
	
	// ClientHandler Setting
	ClntHandler = new ClientHandler;

	// Select Setting
	maxfd = ServerSocket;
	FD_ZERO(&ori_reads);
	FD_SET(ServerSocket, &ori_reads);

	// MySQL connection Setting
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("SmartHome");
}

Handler::~Handler() {
	delete EventStmt;
	delete CurrentStmt;
	delete OtpStmt;
	delete con;
}

// Initialize MySQL
void Handler::InitSQL() {
	// Initialize Database table
	InitStmt = con->createStatement();
	InitStmt->execute("CREATE TABLE IF NOT EXISTS EventLog(Number int auto_increment primary key, Device text, Status text, Date text)");
	InitStmt->execute("DROP TABLE IF EXISTS OTP");
	InitStmt->execute("CREATE TABLE IF NOT EXISTS OTP(CurrentValue text, Date text)");

	InitStmt->execute("DROP TABLE IF EXISTS CurrentStatus"); 	
	InitStmt->execute("CREATE TABLE IF NOT EXISTS CurrentStatus(Device text, Status text, Date text)");
	// Initialize Table value
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('LED1', 'Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('LED2', 'Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('LED3', 'Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('Dimmer','Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('StreetLamp','Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('GAS','Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('FAN','Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('Door','Unknown')");
	InitStmt->execute("INSERT INTO CurrentStatus(Device, Status) VALUES('Curtain','Unknown')");
	InitStmt->execute("INSERT INTO OTP(CurrentValue, Date) VALUES('Unknown', 'Unknown')");
	// PreparedStatement Setting
	EventStmt = con->prepareStatement("INSERT INTO EventLog(Device, Status, Date) VALUES(?, ?, ?)");
	CurrentStmt = con->prepareStatement("UPDATE CurrentStatus SET Status = ?, Date = ? WHERE Device = ?");
	OtpStmt = con->prepareStatement("UPDATE OTP SET CurrentValue = ?, Date = ?");

	delete InitStmt;
}
	
// Update Current Time
void Handler::UpdateTime() {
	memset(EventTime, 0, sizeof(EventTime));
	// Year-Month-Day-Hour-Minute
	sprintf(EventTime, "%d-%02d-%02d %02d:%02d", tm.Get_year(), tm.Get_mon(), tm.Get_day(), tm.Get_hour(), tm.Get_min());
}

void Handler::OccurGas() {
	SOCKET sock;
	for (int i = 0; i < ClntHandler->GetMaxClnt(); i++) {
		sock = (*ClntHandler)[i];
		if (sock != 0) { 
			Server->send(&MESSAGE::OCCURGAS, sizeof(MESSAGE::OCCURGAS), sock);
		}
	}
}

char* Handler::ExtractStatus(Device &DevPacket) {
	char *StatusName;

	switch (DevPacket.Class) {
	case DEVICE::CLASS::DIMMER:
		// Write status of dimmer
		switch(DevPacket.Status) {
		case STATUS::DIMMER::LEVEL0:
			StatusName = new char[sizeof("LEVEL0")];
			strcpy(StatusName, "LEVEL0"); break;
		case STATUS::DIMMER::LEVEL1:
			StatusName = new char[sizeof("LEVEL1")];
			strcpy(StatusName, "LEVEL1"); break;
		case STATUS::DIMMER::LEVEL2:
			StatusName = new char[sizeof("LEVEL2")];
			strcpy(StatusName, "LEVEL2"); break;
		case STATUS::DIMMER::LEVEL3:
			StatusName = new char[sizeof("LEVEL3")];
			strcpy(StatusName, "LEVEL3"); break;
		case STATUS::DIMMER::LEVEL4:
			StatusName = new char[sizeof("LEVEL4")];
			strcpy(StatusName, "LEVEL4"); break;
		default:
			StatusName = new char[sizeof("Unknown Status")];
			strcpy(StatusName, "Unknown Status");
		} break;

	case DEVICE::CLASS::CURTAIN:
		// Write status of curtain
		switch(DevPacket.Status) {
		case STATUS::CURTAIN::CLOSE:
			StatusName = new char[sizeof("Close")];
			strcpy(StatusName, "Close"); break;
		case STATUS::CURTAIN::MIDDLE:
			StatusName = new char[sizeof("Middle")];
			strcpy(StatusName, "Middle"); break;
		case STATUS::CURTAIN::OPEN:
			StatusName = new char[sizeof("Open")];
			strcpy(StatusName, "Open"); break;
		default:
			StatusName = new char[sizeof("Unknown Status")];
			strcpy(StatusName, "Unknown Status");
		} break;

	case DEVICE::CLASS::GAS:
		switch(DevPacket.Status) {
		case STATUS::ONOFF::OFF:
			StatusName = new char[sizeof("Off")];
			strcpy(StatusName, "Off"); break;
		case STATUS::ONOFF::ON:
			std::cout << std::endl << "Warnning: Gas Leakage" << std::endl << std::endl;
			OccurGas();
			StatusName = new char[sizeof("On")];
			strcpy(StatusName, "On"); break;
		default:
			StatusName = new char[sizeof("Unknown Status")];
			strcpy(StatusName, "Unknown Status");
		} break;
	
	default:
		// Write status of common
		switch(DevPacket.Status) {
		case STATUS::ONOFF::OFF:
			StatusName = new char[sizeof("Off")];
			strcpy(StatusName, "Off"); break;
		case STATUS::ONOFF::ON:
			StatusName = new char[sizeof("On")];
			strcpy(StatusName, "On"); break;
		default:
			StatusName = new char[sizeof("Unknown Status")];
			strcpy(StatusName, "Unknown Status");
		}
	}

	return StatusName;
}

char* Handler::ExtractClass(Device &DevPacket) {
	char *DeviceName;

	// Write device name
	switch (DevPacket.Class) {
	case DEVICE::CLASS::DOOR:			// Device : Door
		DeviceName = new char[sizeof("Door")];
		strcpy(DeviceName, "Door"); break;
	case DEVICE::CLASS::DIMMER:			// Device : Dimmer
		DeviceName = new char[sizeof("Dimmer")];
		strcpy(DeviceName, "Dimmer"); break;
	case DEVICE::CLASS::STREETLAMP:		// Device : StreetLamp
		DeviceName = new char[sizeof("StreetLamp")];
		strcpy(DeviceName, "StreetLamp"); break;
	case DEVICE::CLASS::CURTAIN:		// Device : Curtain
		DeviceName = new char[sizeof("Curtain")];
		strcpy(DeviceName, "Curtain"); break;
	case DEVICE::CLASS::GAS:			// Device : Curtain
		DeviceName = new char[sizeof("Gas")];
		strcpy(DeviceName, "Gas"); break;
	case DEVICE::CLASS::FAN:			// Device : Fan
		DeviceName = new char[sizeof("Fan")];
		strcpy(DeviceName, "Fan"); break;
	case DEVICE::CLASS::LED:			// Device : LED
		DeviceName = ExtractDevice(DevPacket.Device);
		break;
	default:
		DeviceName = new char[sizeof("Unknown Device")];
		strcpy(DeviceName, "Unknown Device");
	}

	return DeviceName;
}

char* Handler::ExtractDevice(char Dev) {
	char *DeviceName;

	// Write detail device name
	switch (Dev) {
	case DEVICE::LED::LED1:
		DeviceName = new char[sizeof("LED1")];
		strcpy(DeviceName, "LED1"); break;
	case DEVICE::LED::LED2:
		DeviceName = new char[sizeof("LED2")];
		strcpy(DeviceName, "LED2"); break;
	case DEVICE::LED::LED3:
		DeviceName = new char[sizeof("LED3")];
		strcpy(DeviceName, "LED3"); break;
	default:
		DeviceName = new char[sizeof("Unknown Device")];
		strcpy(DeviceName, "Unknown Device");
	}
	
	return DeviceName;
}

void Handler::ProcOTP(SOCKET sock) {
	char OTP[OTPSIZE] = {0, };
	try { Server->recv(OTP, sizeof(OTP), sock); }
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcOTP(), receive failed" << std::endl;
		e.What();
		std::cerr << std::endl;
	}

	// Print packet information
	std::cout << std::endl;
	std::cout << "=====OTP Packet=====" << std::endl;
	std::cout << "OTP : " << OTP << std::endl;
	std::cout << std::endl;

	UpdateTime();
	try {
		// Logging Event
		EventStmt->setString(1, "OTP");
		EventStmt->setString(2, OTP);
		EventStmt->setString(3, EventTime);
		EventStmt->execute();
	
		// Update OTP value
		OtpStmt->setString(1, OTP);
		OtpStmt->setString(2, EventTime);
		OtpStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcOTP(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}
}

void Handler::ProcDevice(SOCKET sock) {
	Device DevPacket;
	memset(&DevPacket, 0, sizeof(DevPacket));
	try { Server->recv(&DevPacket, sizeof(DevPacket), sock); }
	catch (TcpServerSock::Exception &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcDevice(), receive failed" << std::endl;
		e.What();
		std::cerr << std::endl;
	}
	
	char *Device = ExtractClass(DevPacket);
	char *Status = ExtractStatus(DevPacket);

	// Print packet information
	std::cout << std::endl;
	std::cout << "=====Device Packet=====" << std::endl;
	std::cout << "Device : " << Device << std::endl;
	std::cout << "Status : " << Status << std::endl;
	std::cout << std::endl;

	UpdateTime();
	try {
		// Logging Event
		EventStmt->setString(1, Device);
		EventStmt->setString(2, Status);
		EventStmt->setString(3, EventTime);
		EventStmt->execute();
		
		// Update current device status(CurrentStatus table)
		CurrentStmt->setString(1, Status);
		CurrentStmt->setString(2, EventTime);
		CurrentStmt->setString(3, Device);
		CurrentStmt->execute();
	} catch (sql::SQLException &e) {
		std::cerr << std::endl;
		std::cerr << "Error occured in ProcDevice(), SQL error" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << std::endl;
	}

	delete[] Device;
	delete[] Status;
}
 
void Handler::ProcNewConn() {
	try { Server->accept(); }
	catch(TcpServerSock::Exception e) {
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
}

void Handler::ProcDisconn(SOCKET fd) {
	std::cout << "Disconnected!" << std::endl;
	close(fd);
	ClntHandler->RemoveClnt(fd);
	FD_CLR(fd, &ori_reads);
} 

void Handler::Run() {
	InitSQL();
	while(Power) {
		reads = ori_reads;
		select(maxfd + 1, &reads, NULL, NULL, NULL);
		// If client request new connectfdon
		if (FD_ISSET(ServerSocket, &reads)) {
			ProcNewConn();
			continue;
		}
		// Check changed FD 
		for (int fd = 3; fd <= maxfd; fd++) {
			// If the FD fds readable
			if (FD_ISSET(fd, &reads)) {
				char type = 0;
				int res;

				try { res = Server->recv(&type, sizeof(type), fd); }
				catch (TcpServerSock::Exception e) {
					std::cerr << std::endl;
					std::cerr << "Error occured in Run(), receive failed" << std::endl;
				    e.What();
					std::cerr << std::endl;
				}

				// If client request close connectfdon
				if (res == CLOSE)
					ProcDisconn(fd);

				// If client send message
				else {
					if (type >= '0') type -= '0';

					switch (type) {
					case MESSAGE::OTP:		// OTP Message
						ProcOTP(fd); break;
					case MESSAGE::DEVICE:	// Device Message
						ProcDevice(fd); break;
					case MESSAGE::ANDROID:	// Android Message
						std::cout << std::endl;
						std::cout << "=====Android=====" << std::endl;
						std::cout << "FD : " << fd << std::endl;
						std::cout << std::endl;
						ClntHandler->AddClnt(fd); break;
					default:
						std::cerr << "Invalid Packet" << std::endl;
					}
				}
			}
		}
	}
}

void Handler::Stop() { Power = false; }
void Handler::Start() { Power = true; }

