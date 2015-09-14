#include "utils.h"
#include "SocketServer.h"
#define MAX_LEN 1024

void SocketCleanUp();
int main(int argc, char **argv)
{
	if(argc != 3)
	{
		//invlaid usage
		cout<<"Usage "<<argv[0]<<" <server ip address> <server port>"<<endl;		
		exit(1);
	}

	WSADATA wsaData;
	    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
		SocketCleanUp();
        return 1;
    }
	
	//Enter code only after initializing winsock
	struct addrinfo *result = NULL, hints;
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
		SocketCleanUp();
        exit(1);
    }

	    // Create a SOCKET for connecting to server
	SOCKET clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (clientSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
		SocketCleanUp();
        exit(1);
    }

    // Connect to server.
	iResult = connect( clientSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
		SocketCleanUp();
		exit(1);
    }

	freeaddrinfo(result);

	//send Authentication information
	std::string userName;
	cout<<"Enter username: ";
	getline(std::cin, userName);

	cout<<endl;
	cout<<"Enter password: ";
	std::string passwd;
	getline(std::cin, passwd);
	std::string authMsg = userName + "\t" + passwd;

	iResult = send( clientSocket, authMsg.c_str(), authMsg.length(), 0 );
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		SocketCleanUp();
		return 1;
    }

	char buffer[MAX_LEN];
	//wait for response. If the response is not success disconnect
	int recvLen = recv(clientSocket, buffer, MAX_LEN-1, 0);
	buffer[recvLen] = '\0';
	if(strcmp(buffer, "success") == 0)
	{
		cout<<"Successfully authenticated"<<endl;
	}
	else
	{
		cout<<"Authentication failed."<<endl;
		SocketCleanUp();
		exit(1);
	}

	recvLen = recv(clientSocket, buffer, MAX_LEN-1, 0);
	buffer[recvLen] = '\0';
	if(strncmp(buffer, "startfile", 9) == 0)
	{
		//received password file.
		char *tmpLine = strtok(buffer, "\n");
		std::ofstream pwd_file;
		pwd_file.open("passwds.pwd");
		tmpLine = strtok(NULL, "\n");
		while(strcmp(tmpLine, "endfile") != 0)
		{
			pwd_file << tmpLine;
			pwd_file << "\n";
			tmpLine = strtok(NULL, "\n");
			if(!tmpLine)
			{
				recvLen = recv(clientSocket, buffer, MAX_LEN-1, 0);
				buffer[recvLen] = '\0';
				tmpLine = strtok(NULL, "\N");
			}
		}
		pwd_file.close();
		cout<<"received password file"<<endl;
	}

	//start a socket here
	SocketServer Server(12345);
	Server.Start();
	cout<<"press enter to stop server"<<endl;
	getchar();
	Server.Stop();
	//close connection once done
	closesocket(clientSocket);
	//any code should be above this part.
	SocketCleanUp();
	return 0;
}

void SocketCleanUp()
{
	//de initialize winsock
	WSACleanup();
}