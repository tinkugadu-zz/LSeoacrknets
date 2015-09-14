#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include <map>

//windows native headers to be included
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//library to handle windows thread
#define MAX_LEN 1024

#define PWD_FILE "/passwds.pwd"

using namespace std;
enum AuthResponse
{
    AUTH_ERROR,
    UNKNOWN_USER,
    WRONG_PASSWORD,
    SUCCESS
};

bool GetUsersFromFile(std::map<std::string, std::string> &userMap);
AuthResponse AuthenticateClient(char *buffer, std::string &clientName);
#endif //UTILS_H
