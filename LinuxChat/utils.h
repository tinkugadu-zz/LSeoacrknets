#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <map>
//for network related operation
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h> //for threads

#include <string.h> //for memset and other string operations
using namespace std;

#define PWD_FILE "/passwds.pwd"

enum AuthResponse
{
    ERROR,
    UNKNOWN_USER,
    WRONG_PASSWORD,
    SUCCESS
};

bool GetUsersFromFile(std::map<std::string, std::string> &userMap);

//creates a default password file if none exists
void CreatePasswordFile();
//checks if the password file exists
bool IsPasswordFileExist();


void UpdatePasswordFile(std::map<std::string, std::string> userMap);

AuthResponse ChangePassword(std::string userName, std::string passwd);

void getIpAddressPort(struct sockaddr* addr, char* ipAddr, int *portNum);

AuthResponse AuthenticateClient(char *buffer, std::string &clientName);

#endif //UTILS_H
