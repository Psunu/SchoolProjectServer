#ifndef __CUSTOMER_DECL_H
#define __CUSTOMER_DECL_H

typedef SOCKET int;

/* Message packet structure */
typedef struct {
	char resident;				// Wehther resident or not
	char status;				// Car status
	char id[CARIDSIZE + 1];		// Car id
	char location[LOCSIZE + 1];	// Car location
} packet_message;

const int CARIDSIZE = 16;
const int 10MIN = 600;			// 600 seconds = 10minutes
#endif
