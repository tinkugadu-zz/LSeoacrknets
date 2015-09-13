#include "SocketServer.h"

//start the socket server. returns true if successfully started
bool SocketServer::Start()
{
    cout<<"starting server....."<<endl;
    _serverStarted = true;
    return _serverStarted;
}

void SocketServer::Stop()
{
    if(!_serverStarted)
    {
        _serverStarted = false;
    }
    cout<<"stopping server.... "<<endl;
}

