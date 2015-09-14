#include "SocketServer.h"

bool SocketServer::Start()
{
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

	struct addrinfo *result = NULL;
	// Resolve the server address and port
	int iResult = getaddrinfo(NULL, _port.c_str(), &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);        
		return false;
    }
	//creat a socket to listen for connections
	
	_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (_listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
		return false;
    }

	_serverStarted = true;
	cout<<"starting socket server..."<<endl;
	return _serverStarted;
}

void SocketServer::Stop()
{
	if(_serverStarted)
	{
		_serverStarted = false;

		//close existing listening socket
		closesocket(_listenSocket);
		_listenSocket = INVALID_SOCKET;
	}
	cout<<"Socket server stopped!"<<endl;
}