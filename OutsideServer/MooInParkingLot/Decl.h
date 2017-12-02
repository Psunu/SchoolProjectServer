#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;			// socket data type

/* Message packet size*/
const int CARIDSIZE = 16;	// Car id length

/* Login packet size */
const int IDSIZE = 16;		// Login ID length
const int PWSIZE = 16;		// Login Password length

const int DEFAULT_FEE = 100;	// Default parking fee
const int FEE_TIME = 10;		// 10 Minute
const int FEE_MONEY = 100;		// 100 Won

const int MIN = 60;

/* Packet constant */
namespace PACKET {
	/* Data Type */
	namespace TYPE {
		enum {
			MESSAGE,	// Message packet
			LOGIN		// Login packet
		};
	}

	/* Whether Resident or not */
	namespace RESIDENT {
		enum {
			NO,
			YES
		};
	}

	/* Car status */
	namespace INOUT {
		enum {
			OUT,
			IN
		};
	}

}

/* Message packet structure */
typedef struct {
	char resident;				// Wehther resident or not
	char status;				// Car status
	char id[CARIDSIZE + 1];		// Car id
} packet_message;

/* Login packet structure */
typedef struct {
	char id[IDSIZE + 1];		// ID
	char pw[PWSIZE + 1];		// Password
} packet_login;

/* Parking fee  packet structure */
typedef struct {
	char id[CARIDSIZE + 1];		// Car id
	int fee;					// Parking fee
} packet_fee;

const int CLOSE = 0;		// Close signal

#endif
