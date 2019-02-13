/*
 * CPSC 612 Spring 2019
 * HW1
 * by Chengyi Min
 */
#include <Windows.h>
class Utilities {
public:
	int request_parse(char *host, char *request, char* str);
	DWORD DNSParse(char * host, struct sockaddr_in &server);
	int winsockInitialize();
	long long link_count(char* host, char* fileBuf, long long fileSize);
};
