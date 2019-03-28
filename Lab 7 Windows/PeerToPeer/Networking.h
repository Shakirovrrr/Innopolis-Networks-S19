#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

void GetIPAndPort(SOCKET socket, char * ipAddr, int * port);

void GetMyIPAndPort(char * ipAddr, int * port);

SOCKET InitTCPServer(int port);

SOCKET InitTCPClient(char * ipAddr, int port);
