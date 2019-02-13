/* Building Request and receiving response
/*
 * CPSC 612 Spring 2019
 * HW1
 * by Chengyi Min
 */
#include "pch.h"



//MySocket::MySocket()
//{
//	buf = new char[TRUNC];
//	host = new char[MAX_HOST_LEN];
//	request = new char[MAX_REQUEST_LEN];
//	port = 80;
//}

//MySocket::MySocket(SOCKET sock)
//{
//	buf = new char[TRUNC]; // start with 8 KB	
//	this->sock = sock;
//}

//MySocket::~MySocket()
//{
//	delete &buf;
//	delete &host;
//	delete &request;
//	//delete &method;
//	//delete &pageStart;
//	//delete &source;
//}

void MySocket::clearBuf()
{
	if (strlen(buf) > 32000)
		delete(buf);
}

long long MySocket::Read( SOCKET &sock, INT64 max)
{
	long long curPos = 0;
	long long allocatedSize = TRUNC;
	// set timeout to 10 seconds
	TIMEVAL *timeout = new TIMEVAL;
	timeout->tv_sec = 10;
	timeout->tv_usec = 0;
	clock_t start;
	clock_t end;
	clock_t duration;
	
	while (true)
	{
		int ret = -1;
		start = clock();
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(sock, &fd);
		// wait to see if socket has any data (see MSDN)
		if ((ret = select(0, &fd, NULL, NULL, timeout)) > 0)
		{
			end = clock();		
			duration = 1000.0*(end - start) / (double)(CLOCKS_PER_SEC);
			long long usecTimeout = timeout->tv_sec * 1000000 + timeout->tv_usec - duration*1000;
			timeout->tv_sec = usecTimeout/1000000;
			timeout->tv_usec = usecTimeout - 1000000 * timeout->tv_sec;
			FD_ZERO(&fd);
			FD_SET(sock, &fd);
			// new data available; now read the next segment
			int bytes = recv(sock, buf + curPos, allocatedSize - curPos, 0);
			if (bytes < 0)
			{
				//printf("failed with %d on recv\n", WSAGetLastError());
				break;
			}
			if (bytes <= 0)
			{
				// NULL-terminate buffer
				return curPos; // normal completion
			}
			if (bytes < TRUNC)
			{
				buf[curPos + bytes] = 0;
			}
			curPos += bytes; // adjust where the next recv goes
			if (max != 0 && curPos > max) //exceeding max
			{
				//printf("failed with exceeding max\n");
				break;
			}

			if (allocatedSize - curPos < THRESHOLD)
			{
				// resize buffer; besides STL, you can use
				// realloc(), HeapReAlloc(), or memcpy the buffer
				// into a bigger array
				char *temp = (char*)realloc(buf, allocatedSize * 2); //changed the allocation increment and THRESHOLD
				if (!temp)
				{
					printf("not enough space\n");
					return -1;
				}
				allocatedSize *= 2;
				//free(buf);
				//memcpy(temp, buf, allocatedSize);
				buf = temp;
			}
		}
		else if (ret == 0) //time limit expired
		{
			//printf("failed with slow download\n");
			break;
		}
		else //wsa errors
		{
			//printf("failed with %d on recv\n", WSAGetLastError());
			break;
		}
	}
	delete(timeout);
	return -1;
}
