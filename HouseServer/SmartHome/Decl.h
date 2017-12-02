typedef int SOCKET;

const int OTPSIZE = 6;

/*Message Type*/
namespace MESSAGE {
	enum {
		OTP,
		DEVICE,
		ANDROID
	};
	
	const char OCCURGAS = 1;
}

/*Device Type*/
namespace DEVICE {
	namespace CLASS {
		enum {
			DOOR = 1,		//00000001 (1)
			LED,			//00000010 (2)
			DIMMER,			//00000011 (3)
			STREETLAMP,		//00000100 (4)
			CURTAIN,		//00000101 (5)
			GAS,			//00000110 (6)
			FAN				//00000111 (7)
		};
	}
	// LED
	namespace LED {
		enum {
			LED1,
			LED2,
			LED3
		};		
	}
}

/*Status Type*/
namespace STATUS {
	namespace ONOFF {
		enum {
			OFF,
			ON
		};
	}
	// Dimmer
	namespace DIMMER {
		enum {
			LEVEL0,
			LEVEL1,
			LEVEL2,
			LEVEL3,
			LEVEL4
		};
	}
	// Curtain
	namespace CURTAIN {
		enum {
			CLOSE,
			MIDDLE,
			OPEN
		};
	}
}

// Packet Structure
typedef struct {
	char Class;
	char Device;
	char Status;
} Device;

/*Message Status*/
const int CLOSE = 0;
