#include <iostream>
#include <cstring>
#include "Handler.h"

using namespace std;

int main(void) {
	std::cout << "Server Start!" << std::endl;
	Handler handler;
	handler.Run();
}
