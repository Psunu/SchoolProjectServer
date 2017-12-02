#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;				// socket data type
const int NAMESIZE = 9;			// Name length
const int STUDENTIDSIZE = 4;	// Student ID length
const int IDSIZE = 16;			// Login ID length
const int PWSIZE = 16;			// Login Password length
const int DATESIZE = 5;			// MM_DD Format

namespace DEVICETYPE {
	enum {
		RASPBERRY = 1,
		ANDROID
	};
}

namespace DATATYPE {
	enum {
		COMMON,
		LOGIN,
		FORCE_UPDATE
	};
}

namespace MESSAGE {
	namespace STATUS {
		enum {
			LATENESS,
			ABSENCE,
			EARLYLEAVE,
			GOOUT,
			ETC,
			NOCARD,
			ATTEND
		};
	}

	// STATUS : LATENESS, ABSENCE, EARLYLEAVE
	namespace COMMON {
		enum {
			DISEASE,
			RECOGNITION,
			WITHOUT
		};
	}

	// STATUS : GOOUT
	namespace INOUT {
		enum {
			OUT,
			IN
		};
	}

	// STATUS : ETC
	namespace ETC {
		enum {
			HEALTH,
			ETC
		};
	}

	namespace WEEK {
		enum {
			MON,
			TUE,
			WED,
			THU,
			FRI
		};
	}

	namespace PERIOD {
		enum {
			FIRST = 1,
			SECOND,
			THIRD,
			FOURTH,
			FIFTH,
			SIXTH,
			SEVENTH
		};
	}
}

namespace PRIORITY {
	enum {
		NOPRIORITY,
		FIRST,
		SECOND,
		THIRD,
		FOURTH,
		FIFTH,
		SIXTH,
	};
}

/* Packet Structure */
// Common packet Structure
typedef struct {
	char id[STUDENTIDSIZE + 1];
	char status;
	char reason;
	char week;
	char period;
} packetCommon;

// Login packet Structure
typedef struct {
	char id[IDSIZE + 1];
	char password[PWSIZE + 1];
} packetLogin;

// force update packet Structure
typedef struct {
	char id[STUDENTIDSIZE + 1];
	char status;
	char reason;
	char date[DATESIZE + 1];
	char period;
} packetForce;

/*Message Status*/
const int CLOSE = 0;		

#endif
