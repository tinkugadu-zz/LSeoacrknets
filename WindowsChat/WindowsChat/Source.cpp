#include "utils.h"
#include "SocketServer.h"

void SocketCleanUp();

struct ClientDetails
{
	SOCKET sockFd;
	char Name[128];
};

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
		pwd_file.open(PWD_FILE);
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

	//start client Input thread for chatting
	DWORD dwordChatThr;	
	ClientDetails *details = new ClientDetails();
	details->sockFd = clientSocket;
	strcpy(details->Name, userName.c_str());
	HANDLE chatThr = CreateThread(NULL, 0, ChatThread, details, 0, &dwordChatThr);
	if(chatThr == NULL)
	{
		perror("error creating chatting thread for client");
		SocketCleanUp();
		exit(1);
	}
	
	//start a socket here
	SocketServer Server("12345");
	if(!Server.Start())
	{
		//failed to start server. closing server.
		SocketCleanUp();
		exit(1);
	}
	cout<<"press enter to stop server"<<endl;
	WaitForMultipleObjects(1, &chatThr, TRUE, INFINITE);
	Server.Stop();	
	//any code should be above this part.
	SocketCleanUp();
	return 0;
}

void SocketCleanUp()
{
	//de initialize winsock
	WSACleanup();
}

bool GetUsersFromFile(std::map<std::string, std::string> &userMap)
{   
    std::string fileName = PWD_FILE;    
    std::fstream pwd_file(fileName.c_str());
    if(!pwd_file.is_open())
    {
        perror("Error opening password file");
        return false;
    }
    std::string line;
    while(getline(pwd_file, line))
    {
        int ind = line.find('\t');
        if(ind == 0 || ind == EOF)
        {
            continue;
        }
        userMap.insert(std::pair<std::string, std::string>(line.substr(0, ind),
                                    line.substr(ind+1, line.length()-1-ind)));
    }
    pwd_file.close();
    if(userMap.size() == 0)
    {
        return false;
    }
    return true;
}

AuthResponse AuthenticateClient(char *buffer, std::string &clientName)
{
    //authenticate user and send response
    std::map<std::string, std::string> userMap;
    if(!GetUsersFromFile(userMap))
    {
		return AUTH_ERROR;
    }
    char *user = strtok(buffer, "\t");
    clientName = user;
    char *passwd = strtok(NULL, "\t");
    std::string Passwd(passwd);
    std::map<std::string, std::string>::iterator it = userMap.find(clientName);
    //check if username exists
    if(it == userMap.end())
    {
        return UNKNOWN_USER;
    }
    //if passwords dont match, wrong password
    if(it->second != Passwd)
    {
        return WRONG_PASSWORD;
    }
    return SUCCESS;
}

DWORD WINAPI ChatThread( LPVOID lpParam )
{
	std::string line;
	ClientDetails *details = (ClientDetails *)lpParam;
	
	while(true)
	{
		cout<<details->Name<<" : ";
		getline(std::cin, line);
		if(line.length() == 0)
		{
			break;
		}
		//send to server
		int len = send( details->sockFd, line.c_str(), line.length(), 0 );
	}
	//close connection once done
	closesocket(details->sockFd);
	return 0;
}