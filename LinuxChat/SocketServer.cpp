#include "SocketServer.h"

//start the socket server. returns true if successfully started
bool SocketServer::Start()
{
    struct sockaddr_in serv_addr;

//create socket to listen forclients
    _listenSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_listenSockFd == -1)
    {
        perror("error creating listening socket");
        return false;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    //create server address for the socket server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(_port);
    cout<<"starting server.....";
//bind the address to socket server listening socket.
    bind(_listenSockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

//start listening for the windows client to start chatting
//we are only accepting one connection at a time
    listen(_listenSockFd, 1);

    //start a thread to accept new connections.
    pthread_t listenThr;
    _serverStarted = true;
    if(pthread_create(&listenThr, NULL, listeningThread, this) != 0)
    {
        perror("error creating listening thread");
        _serverStarted = false;
        return false;
    }
    cout<<"started"<<endl;
    return _serverStarted;
}

void SocketServer::Stop()
{
    if(!_serverStarted)
    {
        _serverStarted = false;
        //close listening socket as well
        close(_listenSockFd);
    }
    cout<<"stopping server.... "<<endl;
}

//this thread starts accepting connections
void *SocketServer::listeningThread( void *ptr )
{
    SocketServer *server = (SocketServer *)ptr;
    while(server->IsRunning())
    {
        cout<<"server is accepting connections"<<endl;
        sleep(2);
    }
}
