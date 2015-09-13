#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//this is a TCP socket server that listens on configured port
class SocketServer
{
    int _port;
    bool _serverStarted;
    int _numConn;
    int _listenSockFd;
    static void *listeningThread( void *ptr );

public:
    SocketServer(int port): _port(port),
      _serverStarted(false), _numConn(0)
    {
        cout<<"initializing socket server"<<endl;
    }

    bool Start();
    void Stop();
    bool IsRunning() {return _serverStarted; }
    ~SocketServer()
    {
        cout<<"socketserver destructor called"<<endl;
    }
};
#endif //SOCKETSERVER_H
