#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;			// socket data type
const int NAMESIZE = 9;	// Name length
const int IDSIZE = 16;
const int PWSIZE = 16;

namespace DEVICETYPE {
	enum {
		RASPBERRY,
		ANDROID
	};
}

namespace DATATYPE {
	enum {
		COMMON = 1,
		LOGIN
	};
}

namespace MESSAGE {
	namespace STATUS {
		enum {
			IN,
			LESSON,
			OUTING,
			BUSINESSTRIP,
			OFFWORK
		};
	}
}

/* Packet structure */
// Common packet Structure
typedef struct {
	char name[NAMESIZE + 1];
	char status;
} packetCommon;

// Login packet Structure
typedef struct {
	char id[IDSIZE + 1];
	char password[PWSIZE + 1];
} packetLogin;

// Login success packet Structure
typedef struct {
	char flag;
	char name[NAMESIZE + 1];
} packetLoginSuccess;

/*Message Status*/
const int CLOSE = 0;		

#endif
