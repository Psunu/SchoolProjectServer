#ifndef __SUNWOO_DECLARE_H
#define __SUNWOO_DECLARE_H

typedef int SOCKET;			// socket data type
const int NAMESIZE = 9;		// Name length

namespace DATATYPE {
	enum {
		SHOW_AMOUNT,
		SHOW_PICTURE,
		FEED
	};
}

namespace MESSAGE {
	namespace AMOUNT {
		enum {
			LOW,
			MIDDLE,
			LOT
		};
	}

	namespace TYPE {
		enum {
			FEED1,
			FEED2,
			FEED3,
			WATER
		};
	}
}

/* Packet structure */
typedef struct {
	char feed1_amount;
	char feed2_amount;
	char feed3_amount;
	char water_amount;
} packetShowAmount;

typedef struct {
	char feed;
	char feed_amount;
} packetFeed;

/*Message Status*/
const int CLOSE = 0;		

#endif
