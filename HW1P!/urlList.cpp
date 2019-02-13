/*
 * CPSC 612 Spring 2019
 * HW1
 * by Chengyi Min
 */
#include "pch.h"

int URLRead(char *source, struct sockaddr_in &server, MySocket &mysock, INT64 &numberOfLinks, INT64 &pageSize, bool robot = FALSE, INT64 maxRecvSize = 0);


class StatusCodeStore {
public:
	INT64 nxx[7];
};


class Parameters {
public:
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;
	queue <char*> links;
	INT64 activeThreads;
	INT64  extracted;
	INT64  successDNS;
	INT64  robotPassed;
	INT64  crawledURLs;
	INT64  totalLink;
	INT64 pageSize;
	StatusCodeStore scs;
	CRITICAL_SECTION cs;
	CRITICAL_SECTION sh;
	CRITICAL_SECTION sip;
	//char* url;
	unordered_set<DWORD> seenIPs;
	unordered_set<string> seenHosts;
	clock_t start_time;
	clock_t end_time;
};


UINT stats(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	clock_t past_time = p->start_time;
	int lastPageCount = 0;
	int lastSize = 0;
	while (true)
	{
		p->end_time = clock();
		clock_t duration = 1000.0*(p->end_time - p->start_time) / (double)(CLOCKS_PER_SEC);

		if (duration - past_time >= 2000)
		{
			printf("[%3d]\t%3d Q %3d E %3d H %3d D %3d I %3d R %3d C %3lld L %dK\n", duration / 1000,
				p->activeThreads, p->links.size(), p->extracted, p->seenHosts.size(), p->successDNS,
				p->seenIPs.size(), p->robotPassed, p->crawledURLs, p->totalLink/1000);

			double pageSpeed = (p->crawledURLs - lastPageCount) / (double)(duration - past_time)*1000.0;
			double sizeSpeed = (p->pageSize - lastSize) / (double)(duration - past_time)*1000.0 / 1000000.0;
			if (pageSpeed < 0)
				pageSpeed = 0;
			if (sizeSpeed < 0)
				sizeSpeed = 0;
			printf("\t*** crawling %.1lf pps @ %.1lf Mbps\n", pageSpeed, sizeSpeed*8);

			lastPageCount = p->crawledURLs;
			lastSize = p->pageSize;
			past_time = duration;
		}
		Sleep(1.5);
		if (p->activeThreads <= 0)
			break;
	}
	return 0;
}

UINT crawlingThread(LPVOID pParam)
{

	Utilities ut;
	Parameters *p = ((Parameters*)pParam);
	InterlockedIncrement64(&p->activeThreads);
	MySocket mysock;
	mysock.buf = new char[TRUNC];
	mysock.host = new char[MAX_HOST_LEN];
	mysock.request = new char[MAX_REQUEST_LEN];
	char actualRequest[MAX_REQUEST_LEN];
	char robots[] = "/robots.txt";
	char methodHead[] = "HEAD";
	char methodGet[] = "GET";

	while (true)
	{
		mysock.port = 80;
		INT64 nLinks = 0;
		INT64 pageSize = 0;
		//Pop out a url
		// Critical Section
		EnterCriticalSection(&p->cs);
		if (p->links.size() <= 0)
		{
			//crawling finished
			LeaveCriticalSection(&p->cs);
			break;
		}
		char *url = p->links.front();
		p->links.pop();

		//printf("URL: %s\n", url);
		LeaveCriticalSection(&p->cs);
		// End of Critical Section

		//source url used to pass to HTMLparser
		char *source = new char[strlen(url) + 1];
		memcpy(source, url, strlen(url) + 1);
		
		//getting host, request and port
		
		mysock.port = ut.request_parse(mysock.host, actualRequest, url);
		InterlockedIncrement64(&p->extracted);
		if (mysock.port < 0)
		{
			delete(source);
			continue;
		}
		//printf("host %s, port %d\n", mysock.host, mysock.port);

		//Extracted
		

		//Host uniqueness check
		////Critical Section
		EnterCriticalSection(&p->sh);
		//printf("\tChecking host uniqueness... ");
		int prevHostSize = p->seenHosts.size();
		p->seenHosts.insert(mysock.host);
		if (p->seenHosts.size() <= prevHostSize)
		{
			delete(source);
			//printf("failed\n");
			LeaveCriticalSection(&p->sh);
			continue;
		}
		//printf("passed\n");
		LeaveCriticalSection(&p->sh);
		////End Critical

		//DNS search		
		DWORD IP;
		struct sockaddr_in server;
		if ((IP = ut.DNSParse(mysock.host, server)) == INADDR_NONE) //dns error
		{
			InterlockedIncrement64(&p->successDNS);
			delete(source);
			continue;
		}
		//DNS number updatate
		InterlockedIncrement64(&p->successDNS);


		////IP check
		////Critical Section
		EnterCriticalSection(&p->sip);
		//printf("\tChecking IP uniqueness... ");
		int prevIPSize = p->seenIPs.size();
		p->seenIPs.insert(IP);
		if (p->seenIPs.size() <= prevIPSize)
		{
			delete(source);
			//printf("failed\n");
			LeaveCriticalSection(&p->sip);
			continue;
		}
		//printf("passed\n");
		//uniqueness check finished
		LeaveCriticalSection(&p->sip);
		////End Critical

		//robot check
		mysock.method = methodHead;
		mysock.request = robots;
		int status_code = URLRead(source, server, mysock, nLinks, pageSize, TRUE, MAX_ROBOT);


		if (status_code / 100 == 4)
		{
			InterlockedIncrement64(&p->robotPassed);
			mysock.method = methodGet;
			mysock.request = actualRequest;
			int page_status_code = URLRead(source, server, mysock, nLinks, pageSize, FALSE, MAX_RECV);
			if ((page_status_code / 100) > 0 && (page_status_code / 100) < 7)
			{
				InterlockedIncrement64(&p->crawledURLs);
				if ((page_status_code / 100) > 1 && (page_status_code / 100) < 6)
					InterlockedIncrement64(&p->scs.nxx[page_status_code / 100]);
				else
				{
					printf("other status:%d", page_status_code);
					InterlockedIncrement64(&p->scs.nxx[0]);
				}
			}

			//Parse page
		}
		
		InterlockedAdd64(&p->totalLink, nLinks);
		InterlockedAdd64(&p->pageSize, pageSize);
		delete(source);
		//delete(&mysock);
	}
	InterlockedDecrement64(&p->activeThreads);
	return 0;
}


long long urlListParse(char * filename, int nThreads)
{
	
	// **************Opening and Reading File from HTMLParser
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	// process errors
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d\n", GetLastError());
		return 0;
	}

	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(hFile, &li);
	// process errors
	if (bRet == 0)
	{
		printf("GetFileSizeEx error %d\n", GetLastError());
		return 0;
	}

	// read file into a buffer
	int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
	DWORD bytesRead;
	// allocate buffer
	char *fileBuf = new char[fileSize];
	// read into the buffer
	bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
	
	// process errors
	if (bRet == 0 || bytesRead != fileSize)
	{
		printf("ReadFile failed with %d\n", GetLastError());
		return 0;
	}

	// done with the file
	CloseHandle(hFile);
	//*****************************
	fileBuf[fileSize] = 0;
	printf("Opened %s with size %d\n", filename,fileSize);
	char * pStart = fileBuf;
	char * pEnd = pStart;
	Utilities ut;
	if (ut.winsockInitialize() == -1)
		return -1;

	// a pointer queue storing starting points of every link

	//For threading communication
	Parameters pParam;
	pParam.activeThreads = 0;
	pParam.start_time = clock();
	InitializeCriticalSection(&pParam.cs);
	InitializeCriticalSection(&pParam.sh);
	InitializeCriticalSection(&pParam.sip);
	while ((pEnd = strstr(pStart, "\r\n")) != NULL )
	{
		*pEnd = 0;

		if (*pStart != '\r' && * pStart != 0 && *pStart != '\n')
		{
			pParam.links.push(pStart);
			pStart = pEnd + 2;
		}	
	}
	if (*pStart != '\r' && * pStart != 0 && *pStart != '\n')
	{
		pParam.links.push(pStart);
	}
	//while (!pParam.links.empty())
	//{
	//	crawlingThread(&pParam);
	//}

	HANDLE *handles = new HANDLE[nThreads];

	HANDLE timer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)stats, &pParam, 0, NULL);

	for (int i = 0; i < nThreads; i++)
	{
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)crawlingThread, &pParam, 0, NULL);
	}
	
	for (int i = 0; i < nThreads; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);

	delete(handles);
	DeleteCriticalSection(&pParam.cs);
	DeleteCriticalSection(&pParam.sh);
	DeleteCriticalSection(&pParam.sip);

	clock_t duration = 1000.0*(pParam.end_time - pParam.start_time) / (double)(CLOCKS_PER_SEC);

	printf("%d threads unfinished \n", pParam.activeThreads);

	printf("Extracted %d URLs @ %d / s\n"
		"Looked up %d DNS names @ %d / s\n"
		"Downloaded %d robots @ %d / s\n"
		"Crawled %d pages @ %d / s(%.2lf MB)\n"
		"Parsed %d links @ %d / s\n",
		pParam.extracted, pParam.extracted * 1000 / duration,
		pParam.successDNS, pParam.successDNS * 1000 / duration,
		pParam.robotPassed, pParam.robotPassed * 1000 / duration,
		pParam.crawledURLs, pParam.crawledURLs * 1000 / duration, pParam.pageSize / 1000000.0,
		pParam.totalLink, pParam.totalLink * 1000 / duration
	);

	printf("HTTP codes : 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n",
		pParam.scs.nxx[2],
		pParam.scs.nxx[3],
		pParam.scs.nxx[4],
		pParam.scs.nxx[5],
		pParam.crawledURLs - pParam.scs.nxx[2] - pParam.scs.nxx[3] - pParam.scs.nxx[4] - pParam.scs.nxx[5]
	);

	return 1;
}