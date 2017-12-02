#ifndef __SUNWOO_CUSTOMER_H
#define __SUNWOO_CUSTOMER_H

#include <pthread.h>
#include <unistd.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include "Decl.h"
#include "Time/Time.h"

class Customer {
private:
	packet_message	packet;
	pthread_t		thread_id;

private:
	void Run();

public:
	Customer(SOCKET,packet_message&);
	~Customer();

	pthread_t	getThreadID() const;
};
#endif
