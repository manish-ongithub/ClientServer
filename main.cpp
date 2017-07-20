// TestWebServer.cpp : Defines the entry point for the console application.
//

#include "WebServer.h"
#include <time.h>
#include <stdarg.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

//using namespace std;

int PrintMessage(std::string format, ...)
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
    std::string newFormat = "\n["+strTime+"]\t"+format+"\n";
    va_start(arg, format.c_str());
        //done = vfprintf(stdout, format.c_str(), arg);
    done = vfprintf(stdout, newFormat.c_str(), arg);
    va_end(arg);

    return done;
}



std::vector<std::string> parseCommand(const char *buffer) {
	std::vector<std::string> command;
	char *token;
	//const char *rest = sline.c_str();
	const char s[2] = " ";
	char *rest = _strdup( buffer);
    /*
    while ((token = strtok(rest, " "))) {
		command.push_back(token);
		printf("%s\n", token);
	}
    */

	/* get the first token */
   token = strtok(rest, " ");

   /* walk through other tokens */
   while( token != NULL )
   {
      command.push_back(token);
      printf( " %s\n", token );
      token = strtok(NULL, " ");
   }




	return command;
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
* carriage return, or a CRLF combination.  Terminates the string read
* with a null character.  If no newline indicator is found before the
* end of the buffer, the string is terminated with a null.  If any of
* the above three line terminators is read, the last character of the
* string will be a linefeed and the string will be terminated with a
* null character.
* Parameters: the socket descriptor
*             the buffer to save the data in
*             the size of the buffer
* Returns: the number of bytes stored (excluding null) */
/**********************************************************************/


bool readyToReceive(int sock, int interval = 10)
{
	printf("\n Ready to receive \n");
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	timeval tv;
	tv.tv_sec = interval;
	tv.tv_usec = 0;
	int retval = select(sock + 1, &fds, 0, 0, &tv);
	printf("\n retval %d\n",retval);
	return (retval == 1);
}

int receiveTimeout(int s,int timeout,std::string &data)
{
	int size_recv, total_size = 0;
	struct timeval begin, now;
	char chunk[CHUNK_SIZE];
	double timediff;

	//fcntl(s, F_SETFL, O_NONBLOCK);
	DWORD SOCKET_READ_TIMEOUT_SEC = timeout * 1000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&SOCKET_READ_TIMEOUT_SEC, sizeof(SOCKET_READ_TIMEOUT_SEC));

	gettimeofday(&begin, NULL);

	while (1) {
		gettimeofday(&now, NULL);
		timediff = now.tv_sec - begin.tv_sec + 1e-6*(now.tv_usec - begin.tv_usec);

		//if you got some data, then break after timeout
		if (total_size > 0 && timediff > timeout)
			break;
		else if (timediff > timeout * 2)
			break;

		memset(chunk, 0, CHUNK_SIZE);
		if ((size_recv = recv(s, chunk, CHUNK_SIZE, 0))<0) {
			//if nothing was received wait a little before trying again 0.1 secs.
			//usleep(100000);
			Sleep(100);
		}
		else {
			total_size += size_recv;
			//printf("%s", chunk);
			data.append(chunk);
			gettimeofday(&begin, NULL);
		}

	}
	return total_size;
}

std::string recv_timeout(int s, int SOCKET_READ_TIMEOUT_SEC) {
	DWORD timeout = SOCKET_READ_TIMEOUT_SEC * 1000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	std::string data;
	char rx_tmp[512] = { 0 };

	int recv_size = recv(s, rx_tmp, CHUNK_SIZE - 1, 0);
	if (recv_size == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAETIMEDOUT)
		{

		}
			//...
	}
	else
	{
		data.append(rx_tmp);
	}
	return data;
}

int get_line(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}

void sendOk(int client_sock){

    PrintMessage("sending ok");
    char buff[24] = {0};
    snprintf(buff,24,"%s","OK");
    int n = send(client_sock, buff, strlen(buff), 0);
    if(n != SOCKET_ERROR){
            PrintMessage("Sent %d bytes ",n);
    }
    else
    {
        int iError = WSAGetLastError();
        if (iError == WSAEWOULDBLOCK)
             PrintMessage("recv failed with error: WSAEWOULDBLOCK\n");
        else
             PrintMessage("recv failed with error: %d", iError);

		PrintMessage("error encountered %d returning... ",n);
    }
}

void receiveObject(int client_sock) {

	PrintMessage("inside receiveObject");
	int received_int = 0;
    char buf[24] = {0};

    //int return_status = recv(client_sock, (char*)&received_int, sizeof(received_int),0);
    int return_status = recv(client_sock, buf, sizeof(buf),0);
	PrintMessage("inside receiveObject return status %d",return_status);

	if (return_status != SOCKET_ERROR) {
        received_int = atoi(buf);
		//printf("\n OBJECT SIZE = %d\n", ntohl(received_int));
        PrintMessage(" OBJECT SIZE = %d", received_int);
	}
	else {
		// Handling erros here
		int iError = WSAGetLastError();
        if (iError == WSAEWOULDBLOCK)
             PrintMessage("recv failed with error: WSAEWOULDBLOCK\n");
        else
             PrintMessage("recv failed with error: %d\n", iError);

		PrintMessage("\n error encountered %d\n returning... \n",return_status);

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
			PrintMessage("\n binn_open failed \n");
			return;
		}
         char  strId[] = "id";
         char  strName[] = "name";
         char  strPrice[] = "price";

		id = binn_object_int32(obj, strId);
		name = binn_object_str(obj, strName);
		price = binn_object_double(obj, strPrice);

		PrintMessage("id - %d, name - %s, price -  %f",id,name,price);

		binn_free(obj);  /* releases the binn pointer but NOT the received buf */
	}
	else {
		//handle error
		PrintMessage(" error encountered %d  in receiving object ", return_status);
	}
    sendOk(client_sock);
}

int getIncomingMessageSize(int client_sock){

    char buff[24] = {0};
    int status = recv(client_sock,buff,sizeof(buff),0);
    if (status != SOCKET_ERROR) {
        int nsize  = atoi(buff);
		return nsize;
	}
	else {
		// Handling erros here
		int iError = WSAGetLastError();
        if (iError == WSAEWOULDBLOCK)
             printf("recv failed with error: WSAEWOULDBLOCK\n");
        else
             printf("recv failed with error: %d\n", iError);

		printf("\n error encountered %d\n returning... \n",status);

		return -1;
	}

}

void client_connected(int client_sock, struct sockaddr_in client_name)
{
    struct client_record *pClient = new client();
    pClient->clientHandler(client_sock,client_name);
    delete pClient;
}

int main()
{


	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return 1;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");

	int sock;
	u_short port = 54800;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		printf("\n Socket not created %d\n", sock);
	}
	else
		printf("\n Socket created %d\n", sock);

	struct sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	int bretval = bind(sock, (struct sockaddr *)&name, sizeof(name));
	int namelen = sizeof(name);
	if (getsockname(sock, (struct sockaddr *)&name, &namelen) == -1)
    {
        printf("error in  getsockname");
    }


	//port = ntohs(name.sin_port);

	printf("\nport %d\n", port);

	if (listen(sock, 5)<0)
		printf("error listen\n");

	int client_sock = -1;
	struct sockaddr_in client_name;
	int client_name_len = sizeof(client_name);
	//printf("trying to connect to mongo\n");
	//connectoToMongo();
	printf("\nwaiting for client connection ... \n");
	while (1)
	{
		client_sock = accept(sock, (struct sockaddr *)&client_name, &client_name_len);
		if (client_sock == -1)
		{
			printf("error while accept");
		}
		else
		{
            		std::thread t1(client_connected, client_sock, client_name);
            		t1.detach();

		}

	}
	closesocket(sock);
	WSACleanup();
	return 0;
}
