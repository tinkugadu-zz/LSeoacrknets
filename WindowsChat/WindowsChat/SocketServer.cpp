#include "SocketServer.h"

enum ConnectionState
{
	STOP_CLIENT,
	CONNECTED,
	AUTHENTICATED,
	COMMUNICATION
};
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
		return _serverStarted;
    }

	//creat a socket to listen for connections	
	_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (_listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
		return _serverStarted;
    }

	//bind the socket for TCP connections
	iResult = bind( _listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(_listenSocket);
		return _serverStarted;
    }

	freeaddrinfo(result);

	//start listening for new connections
	iResult = listen(_listenSocket, 1);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(_listenSocket);
		return _serverStarted;
    }
	//set the server started to true for thread purpose
	_serverStarted = true;
	cout<<"starting socket server..."<<endl;
	//start a thread to accept new connections
	DWORD dwordListenThr;
	HANDLE listenThr = CreateThread(NULL, 0, listeningThread, this, 0, &dwordListenThr);
	if(listenThr == NULL)
	{
		//handle thread creation failure
		cout<<"Error starting listening thread"<<endl;
		_serverStarted = false;
		Stop();		
	}
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

DWORD WINAPI SocketServer::listeningThread( LPVOID lpParam )
{
	SocketServer *server = (SocketServer *)lpParam;
	//thread to accept connections from clients
	while(server->IsRunning())
	{
		if(server->NumConn == 0)
		{
			//accept only one connection for this chat program
			struct ClientThreadParam *clientParam = new ClientThreadParam();
			int addrLen = sizeof(clientParam->Addr);
			clientParam->sockFd = accept(server->GetListenSocket(), &(clientParam->Addr), &addrLen);
			if (clientParam->sockFd == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(clientParam->sockFd);
				WSACleanup();
				delete(clientParam);
			}
			else
			{	
				clientParam->server = server;
				//start thread for lcient to continue
				cout<<"accepted connection from client: "<<endl;
				DWORD dwordClientThr;
				HANDLE clientThr = CreateThread(NULL, 0, clientThread, clientParam, 0, &dwordClientThr);
				if(clientThr == NULL)
				{
					cout<<"error creating client thread. Disconnecting client"<<endl;
					closesocket(clientParam->sockFd);
				}
				else
				{
					//successful creating client thread.
					++server->NumConn;
				}
			}//accept socket
		}//if server->NumConn
		Sleep(2000);
	}//while
	cout<<"server stopped accepting connections"<<endl;
	return 0;
}

DWORD WINAPI SocketServer::clientThread( LPVOID lpParam )
{
	ClientThreadParam *clientParam = (ClientThreadParam *)lpParam;
	ConnectionState cur_state = CONNECTED;
	char buffer[MAX_LEN];
	std::string clientName;
	while(clientParam->server->IsRunning() && cur_state != STOP_CLIENT)
	{
		int bufLen = recv(clientParam->sockFd, buffer, MAX_LEN, 0);
		if(bufLen == 0)
		{
			cout<<"no data received from client. Disconnecting !"<<endl;
			cur_state = STOP_CLIENT;
		}
		else
		{
			buffer[bufLen] = '\0';
		}
		switch(cur_state)
		{
			case CONNECTED:
			{
				if(AuthenticateClient(buffer, clientName) == SUCCESS)
				{
					cur_state = COMMUNICATION;
					cout<<"Authentication Successful for "<<clientName<<endl;
				}
				else
				{
					cout<<"Authentication failed. Disconnecting client"<<endl;
					cur_state = STOP_CLIENT;
				}
			}
			break;
			case COMMUNICATION:
			{
				cout<<clientName<<" : "<<buffer<<endl;
			}
			break;
		}//switch state
	}//while connection
	closesocket(clientParam->sockFd);
	//prevent memory leak
	delete(clientParam);
	cout<<"client "<< clientName<<" disconnected!"<<endl;
	return 0;
}