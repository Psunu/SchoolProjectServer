#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;			// socket data type

/* Login packet size */
const int IDSIZE = 16;
const int PWSIZE = 16;

/* Packet constant */
namespace PACKET {
	/* Data Type */
	namespace TYPE {
		enum {
			FLOOR,		// Floor packet
			BUTTON,		// Button packet
			LOGIN		// Login packet
		};
	}

	namespace BUTTON {
		enum {
			DOWN,
			UP
		};
	}
}

/* Message packet structure */
typedef char packetFloor;
typedef struct {
	char floor;
	char button;
} packetButton;

/* Login packet structure */
typedef struct {
	char id[IDSIZE + 1];		// ID
	char pw[PWSIZE + 1];		// Password
} packetLogin;

const int CLOSE = 0;	// Close signal

#endif
