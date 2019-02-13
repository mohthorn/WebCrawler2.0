// HW1.cpp : This file contains the 'main' function. Program execution begins and ends there.
/*
 * CPSC 612 Spring 2019
 * HW1
 * by Chengyi Min
 */

#include "pch.h"

int URLRead(char *source, struct sockaddr_in &server, MySocket &mysock, INT64 &numberOfLinks, INT64 &pageSize, bool robot = FALSE, INT64 maxRecvSize = 0);

long long urlListParse(char * filename, int nThreads);


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		if (argc != 3)
		{
			printf("Usage: executable number_of_threads filename\r\nor\r\nexecutable URL\r\nexiting...\r\n");
			exit(0);
		}
		else
		{
			if (atoi(argv[1]) < 1)
			{
				printf("Threads: %d\r\n", atoi(argv[1]));
				printf("Usage: executable number_of_threads filename\r\nor\r\nexecutable URL\r\nexiting...\r\n");
				exit(0);
			}
		}
	}

	//Read a URL list file
	if (argc == 3)
	{
		urlListParse(argv[2], atoi(argv[1]));
		return 0;
	}

	Utilities ut;
	if (ut.winsockInitialize() == -1)
		return 0;

	printf("URL: %s\n", argv[1]);
	char method[] = "GET";
	MySocket mysock;
	mysock.buf = new char[TRUNC];
	mysock.host = new char[MAX_HOST_LEN];
	mysock.request = new char[MAX_REQUEST_LEN];
	mysock.port = 80;
	//source reserved for page parsing
	char *source = new char[strlen(argv[1]) + 1];
	memcpy(source, argv[1], strlen(argv[1]) + 1);
	mysock.port = ut.request_parse(mysock.host, mysock.request, argv[1]);
	if (mysock.port < 0)
		return (0);
	printf("host %s, port %d, request %s\n", mysock.host, mysock.port, mysock.request);


	mysock.method = method;
	struct sockaddr_in server;
	INT64 nLinks = 0;
	INT64 pageSize = 0;
	if (ut.DNSParse(mysock.host, server) != INADDR_NONE)
		URLRead(source, server, mysock, nLinks, pageSize, FALSE, 0);
	return 0;
}

