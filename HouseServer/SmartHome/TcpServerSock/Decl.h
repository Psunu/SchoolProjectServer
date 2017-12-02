#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

typedef int SOCKET;
const int ERROR = -1;

/*Exception error list*/
namespace ERRLIST {
	enum {
		SOCKERR,
		BINDERR,
		LISTENERR,
		SOCKOPTERR,
		ACCEPTERR,
		SENDERR,
		RECVERR
	};
}
