#include "SocketServer.h"

#define SERVER_PORT 12345
int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout<<"usage "<<argv[0]<<" <ipaddress> port"<<endl;
        exit(1);
    }
    //check if the password file exists, if not create one
    if(!IsPasswordFileExist())
    {
        cout<<"password file does not exist. Creating default file"<<endl;
        CreatePasswordFile();
    }

    SocketServer server(SERVER_PORT);
    server.Start();
//    cout<<"press enter to stop server"<<endl;
    //getchar();
    //server.Stop();
    while(server.NumConn == 0)
    {
        sleep(1);
    }
    //There is a connection to server, so start client
    int clientSockfd;
    char buffer[MAX_LEN];
    struct sockaddr_in serv_addr;
    memset(buffer, '0', sizeof(buffer));

    clientSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSockfd < 0)
    {
        perror("error creating client socket to windows server");
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    //connect to windows server
    if( connect(clientSockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
    cout<<"press enter to connect to windows server"<<endl;
    getchar();
    //Authenticate user
    std::string userName;
    std::string passwd;
    cout<<"enter username: ";
    getline(std::cin, userName);
    cout<<"\nenter password: ";
    getline(std::cin, passwd);
    std::string authMsg = userName + "\t" + passwd;
    int len = write(clientSockfd, authMsg.c_str(), authMsg.length());
    //read response from server for authentication
    len = read(clientSockfd, buffer, MAX_LEN-1);
    buffer[len] = '\0';
    cout<<"received from windows: "<<buffer<<endl;
    if(strcmp(buffer, "success") != 0)
    {
        cout<<"Authentication failed for user "<<userName<<endl;
        close(clientSockfd);
        server.Stop();
        exit(1);
    }
    cout<<"Authentication successful for "<<userName<<endl;

    //send messages to server
    while(true)
    {
        cout<<userName<<" : ";
        getline(std::cin, authMsg);
        if(authMsg.length() == 0)
        {
            break;
        }
        len = write(clientSockfd, authMsg.c_str(), authMsg.length());
    }
    close(clientSockfd);
    server.Stop();
    return 0;
}
