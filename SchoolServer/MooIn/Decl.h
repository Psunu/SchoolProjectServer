#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;			// socket data type

/* Message packet size */
const int DEPTSIZE = 9;		// Department length
const int STIDSIZE = 6;		// Student ID length 330108
const int NAMESIZE = 15;	// Name length
const int RECSIZE = 5;		// Record length

/* Login packet size */
const int IDSIZE = 16;
const int PWSIZE = 16;

/* Department constant */
namespace DEPARTMENT {
	enum {
		CONTROL = 1,
		CIRCUIT,
		COMMUNICATION
	};
}

/* Packet constant */
namespace PACKET {
	/* Data Type */
	namespace TYPE {
		enum {
			MESSAGE,	// Message packet
			LOGIN		// Login packet
		};
	}

	/* Sport Event */	
	namespace EVENT {
		enum {
			RUN50M,		// Run 50M event
			PUSHUP,		// Pushup event
			FLEX		// Time of staying in air event
		};
	}
}

/* Message packet structure */
typedef struct {
	char event;					// Sport Event
	char id[STIDSIZE + 1];		// StudentID
	char name[NAMESIZE + 1];	// Name
	char record[RECSIZE + 1];	// Record
} packet_message;

/* Login packet structure */
typedef struct {
	char id[IDSIZE + 1];		// ID
	char pw[PWSIZE + 1];		// Password
} packet_login;

const int CLOSE = 0;	// Close signal

#endif
