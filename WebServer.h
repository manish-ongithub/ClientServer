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

std::vector<std::string> parseCommand(const char *buffer);

typedef struct client_record {

	std::string clientip;
	std::string clientport;
	int sockfd;
	bool clientloggedin;

    void clientHandler(int client_sock, struct sockaddr_in client_name)
    {
        std::string ip = inet_ntoa(client_name.sin_addr);
        int port = (int)ntohs(client_name.sin_port);
        char buff[20] = { 0 };
        itoa(port,buff,10);
        std::string str_port(buff);
        std::string mapkey = ip + ":" + str_port;
        clientip = ip;
        clientport = str_port;
        sockfd = client_sock;

        printMessage("client details : %s\n", mapkey.c_str());
        printMessage("Inside Thread Function");
        sendOk(client_sock);

        while (1)
        {
            int status;
            char buff[24] = {0};
            status = recv(client_sock,buff,sizeof(buff),0);
            int datalength = atoi(buff);

            printMessage("data length %d",datalength);
            sendOk(client_sock);
            char buff_command[512] = {0};
            status = recv(client_sock,buff_command,datalength,0);
            std::string data(buff_command);
            printMessage("data received %s",data.c_str());
            sendOk(client_sock);
            std::vector<std::string>commands = parseCommand(data.c_str());

            if (commands.size() > 0) {
                if (commands[0] == "logout" || commands[0] == "LOGOUT") {
                    printMessage("\nLogging out client %s\n", mapkey.c_str());
                    closesocket(client_sock);
                    int s = shutdown(client_sock,2);
                    break;
                }
                if (commands[0] == "object" || commands[0] == "OBJECT") {
                    printMessage(" read Object %s",commands[0].c_str());
                    receiveObject(client_sock);
                }
            }
            else
            {
                printMessage("no command received");
            }

        }//END WHILE

    }

    int printMessage(std::string format, ...)
    {
        int done = 0;
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);

        va_list arg;
        std::string strTime(buffer);
        std::string newFormat = "\n["+clientip+":"+clientport+" "+strTime+"]\t"+format+"\n";
        va_start(arg, format.c_str());
            //done = vfprintf(stdout, format.c_str(), arg);
        done = vfprintf(stdout, newFormat.c_str(), arg);
        va_end(arg);

        return done;
    }

    void sendOk(int client_sock)
    {
        printMessage("sending ok");
        char buff[24] = {0};
        snprintf(buff,24,"%s","OK");
        int n = send(client_sock, buff, strlen(buff), 0);
        if(n != SOCKET_ERROR){
                printMessage("Sent %d bytes ",n);
        }
        else
        {
            int iError = WSAGetLastError();
            if (iError == WSAEWOULDBLOCK)
                 printMessage("recv failed with error: WSAEWOULDBLOCK\n");
            else
                 printMessage("recv failed with error: %d", iError);

            printMessage("error encountered %d returning... ",n);
        }
    }

    void receiveObject(int client_sock)
    {

        printMessage("inside receiveObject");
        int received_int = 0;
        char buf[24] = {0};

        //int return_status = recv(client_sock, (char*)&received_int, sizeof(received_int),0);
        int return_status = recv(client_sock, buf, sizeof(buf),0);
        printMessage("inside receiveObject return status %d",return_status);

        if (return_status != SOCKET_ERROR) {
            received_int = atoi(buf);
            //printf("\n OBJECT SIZE = %d\n", ntohl(received_int));
            printMessage(" OBJECT SIZE = %d", received_int);
        }
        else {
            // Handling erros here
            int iError = WSAGetLastError();
            if (iError == WSAEWOULDBLOCK)
                 printMessage("recv failed with error: WSAEWOULDBLOCK\n");
            else
                 printMessage("recv failed with error: %d\n", iError);

            printMessage("\n error encountered %d\n returning... \n",return_status);

            return;
        }
        char buff[1024] = { 0 };
        sendOk(client_sock);
        return_status = recv(client_sock, buff, sizeof(buff),0);
        if (return_status != SOCKET_ERROR) {
            int id;
            char *name;
            double price;
            binn *obj;

            obj = binn_open(buff);
            if (obj == 0) {
                printMessage("\n binn_open failed \n");
                return;
            }
             char  strId[] = "id";
             char  strName[] = "name";
             char  strPrice[] = "price";

            id = binn_object_int32(obj, strId);
            name = binn_object_str(obj, strName);
            price = binn_object_double(obj, strPrice);

            printMessage("id - %d, name - %s, price -  %f",id,name,price);

            binn_free(obj);  /* releases the binn pointer but NOT the received buf */
        }
        else {
            //handle error
            printMessage(" error encountered %d  in receiving object ", return_status);
        }
        sendOk(client_sock);
    }

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
