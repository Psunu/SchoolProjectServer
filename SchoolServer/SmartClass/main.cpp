#include <iostream>
#include <cstring>
#include "Handler.h"

using namespace std;

int main(void) {
	cout << "Server Start!" << endl;
	Handler handler;
	handler.Run();
}
