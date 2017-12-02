MooInParkingLot: main.o
	g++ -o MooInParkingLot main.o Handler.o TcpServerSock.o Time.o -std=c++11 -lmysqlcppconn -lpthread 
	rm *.o

main.o: Handler.o
	g++ -c main.cpp -std=c++11

Handler.o: Handler.h Handler.cpp TcpServerSock.o Time.o
	g++ -c Handler.cpp -std=c++11
	
TcpServerSock.o: TcpServerSock/TcpServerSock.h TcpServerSock/TcpServerSock.cpp
	g++ -c TcpServerSock/TcpServerSock.cpp -std=c++11

Time.o: Time/Time.h Time/Time.cpp
	g++ -c Time/Time.cpp -std=c++11
