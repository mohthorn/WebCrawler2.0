/* Code utilizing HTMLParserBase
/*
 * CPSC 612 Spring 2019
 * HW1P1
 * by Chengyi Min
 */
#include "pch.h"


long long Utilities::link_count(char* host, char* fileBuf, long long fileSize)
{

	HTMLParserBase *parser = new HTMLParserBase;

	int nLinks = 0;
	if (fileBuf != NULL)
	{
		char *linkBuffer = parser->Parse(fileBuf, fileSize, host, (long long)strlen(host), &nLinks);
	}
	// check for errors indicated by negative values
	if (nLinks < 0)
		nLinks = 0;
	delete parser;
	return nLinks;
}



int Utilities::winsockInitialize()
{
	WSADATA wsaData;
	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return FAILURECODE;
	}
	return 1;
}



int Utilities::request_parse(char *host, char* request, char* str)
{
	//printf("\tParsing URL... ");
	long long port;
	char* query = new char[MAX_URL_LEN];
	char* path = new char[MAX_REQUEST_LEN];
	char* scheme = new char[MAX_URL_LEN];
	char* position;
	if (strlen(str) > MAX_URL_LEN)
	{
		//printf("invalid url\n");
		return -1;
	}
	if ((position = strstr(str, "://")) != NULL)
	{
		*position = 0;
		strcpy(scheme, str);
		if (strcmp(scheme, "http"))
		{
			//printf("failed with invalid scheme\n");
			return -1;
		}
		str = position + 3;
	}
	else
	{
		//printf("failed with invalid scheme\n");
		return -1;
	}
	if ((position = strchr(str, '#')) != NULL)
	{
		*position = '\0';
	}
	if ((position = strchr(str, '?')) != NULL)
	{
		if (strlen(position) > MAX_REQUEST_LEN)
		{
			//printf("invalid request\n");
			return -1;
		}
		strcpy(query, position);
		*position = '\0';
	}
	else
	{
		strcpy(query, "");
	}
	if ((position = strchr(str, '/')) != NULL)
	{
		if (strlen(position) > MAX_REQUEST_LEN)
		{
			//printf("invalid request\n");
			return -1;
		}
		strcpy(path, position);
		*position = '\0';
	}
	else
	{
		strcpy(path, "/");
	}
	if ((position = strchr(str, ':')) != NULL)
	{
		sscanf(position + 1, "%lld", &port);
		*position = '\0';
	}
	else
	{
		port = 80;
	}
	if (port < 1 || port >65535)
	{
		//printf("failed with invalid port\n");
		port = -1;
		return port;
	}
	if (strlen(str) > MAX_HOST_LEN)
	{
		//printf("invalid hostname\n");
		return -1;
	}
	strcpy(host, str);

	if ((strlen(path) + strlen(query)) > MAX_REQUEST_LEN)
	{
		//printf("invalid request\n");
		return -1;
	}

	sprintf(request, "%s%s", path, query);

	delete(query);
	delete(path);
	delete(scheme);
	return (int)port;
}

DWORD Utilities::DNSParse(char * host, struct sockaddr_in &server)
{
	struct hostent *remote;
	// DNS varification
	// first assume that the string is an IP address
	//printf("\tDoing DNS... ");
	clock_t start;
	clock_t end;
	clock_t duration;
	start = clock();

	DWORD IP = inet_addr(host);
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(host)) == NULL)
		{
			//printf("failed with %d\n", WSAGetLastError());
			return INADDR_NONE;
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}
	end = clock();
	duration = 1000.0*(end - start) / (double)(CLOCKS_PER_SEC);
	//printf("done in %dms, ", duration);
	//printf("found %s\n", inet_ntoa(server.sin_addr));
	IP = inet_addr(inet_ntoa(server.sin_addr));
	return IP;
}


