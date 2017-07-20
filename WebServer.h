#ifndef WEBSERVER_H_INCLUDED
#define WEBSERVER_H_INCLUDED

#pragma once
#include <iostream>
#include <thread>
#include <map>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <time.h>
#include "binn.h"

#define CHUNK_SIZE 512

typedef struct client_record {
	std::string clientip;
	std::string clientport;
	bool clientloggedin;


}client;

std::map<std::string, client> g_clientMap;




/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

/*
* timezone information is stored outside the kernel so tzp isn't used anymore.
*
* Note: this function is not for Win32 high precision timing purpose. See
* elapsed_time().
*/
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

	return 0;
}

#endif // WEBSERVER_H_INCLUDED
