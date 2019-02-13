/*
 * CPSC 612 Spring 2019
 * HW1P1
 * by Chengyi Min
 */
// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
#pragma once

#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <queue>
#include <string>     
#include <stdio.h>
#include <winsock2.h>
#include <ctime>
#include "HTMLParserBase.h"
#include <SDKDDKVer.h>
#include <tchar.h>
#include <Windows.h>
#include <unordered_set>
#include "utilities.h"
#include "mySocket.h"
using namespace std;

// TODO: add headers that you want to pre-compile here

#endif //PCH_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define THRESHOLD 25000
#define TRUNC 8192
#define FAILURECODE -1
#define MAX_ROBOT 16384
#define MAX_RECV 2097152
#define TIME_LIMIT 10000
#define MAX_HOST_LEN		256
#define MAX_URL_LEN			2048
#define MAX_REQUEST_LEN		2048
#define LINKTHRESHOLD 100