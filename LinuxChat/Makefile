all: LinuxSocket

Helpers.o: utils.h Helpers.cpp
	g++ -c Helpers.cpp

SocketServer.o: utils.h SocketServer.h SocketServer.cpp
	g++ -c SocketServer.cpp

main.o: utils.h SocketServer.h main.cpp
	g++ -c main.cpp

LinuxSocket: Helpers.o SocketServer.o main.o
	g++ -o LinuxSocket -pthread Helpers.o SocketServer.o main.o

clean:
	rm *.o LinuxSocket
