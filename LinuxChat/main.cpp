#include "SocketServer.h"

#define SERVER_PORT 12345
int main()
{
    //check if the password file exists, if not create one
    if(!IsPasswordFileExist())
    {
        cout<<"password file does not exist. Creating default file"<<endl;
        CreatePasswordFile();
    }

    SocketServer server(SERVER_PORT);
    server.Start();
    cout<<"press enter to stop server"<<endl;
    getchar();
    server.Stop();
    return 0;
}
