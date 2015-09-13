#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "utils.h"
#include <sys/socket.h>

class SocketServer;

struct ClientThreadParam
{
    int sockFd;
    struct sockaddr Addr;
    SocketServer* server;
};


//this is a TCP socket server that listens on configured port
class SocketServer
{
    int _port;
    bool _serverStarted;
    int _listenSockFd;
    static void *listeningThread( void *ptr );
    static void *clientThread( void *ptr );

public:
    int NumConn;
    SocketServer(int port): _port(port),
      _serverStarted(false), NumConn(0)
    {
        cout<<"initializing socket server"<<endl;
    }

    bool Start();
    void Stop();
    bool IsRunning() {return _serverStarted; }
    int GetListenSocket() { return _listenSockFd; }
    ~SocketServer()
    {
        cout<<"socketserver destructor called"<<endl;
    }
};
#endif //SOCKETSERVER_H
