SmartHome: main.o
	g++ -o SmartHome main.o Handler.o ClientHandler.o TcpServerSock.o Time.o -lmysqlcppconn 
	rm *.o

main.o: Handler.o
	g++ -c main.cpp

Handler.o: Handler.h Handler.cpp ClientHandler.o TcpServerSock.o Time.o
	g++ -c Handler.cpp

ClientHandler.o: ClientHandler/ClientHandler.h ClientHandler/ClientHandler.cpp
	g++ -c ClientHandler/ClientHandler.cpp
	
TcpServerSock.o: TcpServerSock/TcpServerSock.h TcpServerSock/TcpServerSock.cpp
	g++ -c TcpServerSock/TcpServerSock.cpp

Time.o: Time/Time.h Time/Time.cpp
	g++ -c Time/Time.cpp
