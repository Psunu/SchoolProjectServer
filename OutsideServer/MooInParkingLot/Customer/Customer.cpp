#include "Customer.h"

Customer::Customer(packet_message &packet) : thread_id(0) {
	this->socket = socket;
	this->packet = packet;

	int ret = 0;
	ret = pthread_create(&thread_id, NULL, Run, (void *) &packet);
	if (ret != 0)
		std::cerr << "Error occured in Customer::Customer(), thread create failed" << std::endl;
}

Customer::~Customer() {
	int ret = 0;
	int status = 0;

	ret = pthread_cancel(thread_id);
	if (ret == 0) {
		pthread_join(thread_id, (void**)&status);
		std::cout << "Customer thread canceld, thread id = " << thread_id << std::endl;
	}
	else
		std::cerr << "Error occured in ~Customer(), thread join failed" << std::endl;
}

void *Customer::Run(void *arg) {
	char id[CARIDSIZE] = {0, };
	strcpy(id, arg->id);

	Time tm;
	char EventTime[32] = {0, };

	sql::Driver				*driver;
	sql::Connection			*con;
	sql::PreparedStatement	*stmt;
	
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "korea1234");
	con->setSchema("MooInParkingLot");
	stmt = con->prepareStatement("UPDATE CarList SET Fee = Fee + 100 WHERE ID = ?");

	while(true) {
		sleep(10MIN);
		try {
			stmt->setString(id);
			stmt->execute();
		} catch(sql::SQLException) {
			std::cerr << "Error occured in Customer::Run(), sql error" << std::endl;
			break;
		}
	}

	delete stmt;
	delete con;
}

pthread_t Customer::getThreadID() const { return thread_id; };
