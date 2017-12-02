SmartOffice: main.o
	g++ -o SmartOffice main.o Handler.o  TcpServerSock.o Time.o -lmysqlcppconn 
	rm *.o

main.o: Handler.o
	g++ -c main.cpp

Handler.o: Handler.h Handler.cpp TcpServerSock.o Time.o
	g++ -c Handler.cpp
	
TcpServerSock.o: TcpServerSock/TcpServerSock.h TcpServerSock/TcpServerSock.cpp
	g++ -c TcpServerSock/TcpServerSock.cpp

Time.o: Time/Time.h Time/Time.cpp
	g++ -c Time/Time.cpp
