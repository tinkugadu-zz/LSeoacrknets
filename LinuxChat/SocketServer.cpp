#include "SocketServer.h"

#define MAX_LEN 1024
enum ClientState
{
    STOP_CLIENT,
    CONNECTED,
    AUTHENTICATED,
    COMMAND_MODE,
    COMMUNICATION,
    DISCONNECTED
};

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

    ClientThreadParam *clientParam = new ClientThreadParam();
    socklen_t sockLen = sizeof(clientParam->Addr);

    while(server->IsRunning())
    {
        if(server->NumConn < 1)
        {
            clientParam->sockFd = accept(server->GetListenSocket(),
                            (struct sockaddr*)&(clientParam->Addr), &sockLen);
            clientParam->server = server;
            if(clientParam->sockFd == -1)
            {
                perror("error accepting connection from client");
            }
            else
            {
                ++server->NumConn;
                //create thread for every client connected.
                pthread_t clientThr;
                if(pthread_create(&clientThr, NULL, clientThread, clientParam) != 0)
                {
                    perror("error creating thread for client. disconnecting client");
                    close(clientParam->sockFd);
                    delete(clientParam);
                }
            }
        }
    }
}

void *SocketServer::clientThread( void *ptr )
{
    ClientState cur_state = STOP_CLIENT;
    ClientThreadParam *clientParam = (ClientThreadParam *)ptr;
    char buffer[MAX_LEN];
    char clientAddr[INET_ADDRSTRLEN];
    int clientPort;
    std::string clientName;
    //get Ipaddress and port of client connected
    getIpAddressPort(&(clientParam->Addr), clientAddr, &clientPort);
    cout<<"accepted connection from "<<clientAddr<<" at port : "<<clientPort<<endl;
    cur_state = CONNECTED;

//start while loop to process client communication
    while(clientParam->server->IsRunning() && cur_state != STOP_CLIENT)
    {
        int len = read(clientParam->sockFd, buffer, MAX_LEN-1);
        buffer[len] = '\0';
        //cout<<"debug:: received from client----"<<buffer<<endl;
        switch(cur_state)
        {
            case CONNECTED:
            {
                AuthResponse resp = AuthenticateClient(buffer, clientName);
                if(resp == SUCCESS)
                {
                    cur_state = COMMUNICATION;
                }
                else
                {
                    cout<<"Authentication failure: "<<resp<<"  Disconnecting client"<<endl;
                    cur_state = STOP_CLIENT;
                }
            }
            break;
            case COMMUNICATION:
            cout<<clientName<<" : "<<buffer<<endl;
            break;
        }
    }
    //close socket client if server is stopped
    close(clientParam->sockFd);
    //deallocate heap memory to prevent memory leak.
    delete(clientParam);
    cout<<"client disconnected"<<endl;
}

