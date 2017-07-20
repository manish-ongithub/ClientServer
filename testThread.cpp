#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include "binn.cpp"

#include <sys/resource.h>
#include <unistd.h>


//#define CHUNK_SIZE 512
void print_cpu_time()
{
	struct rusage usage;
	getrusage(RUSAGE_SELF,&usage);
	printf("CPU TIME: %ld.%06ld sec user,%ld.06ld sec system\n",
	usage.ru_utime.tv_sec,usage.ru_utime.tv_usec,usage.ru_stime.tv_sec,usage.ru_stime.tv_usec);
}



int recv_timeout(int s,int timeout)
{
	int size_recv,total_size = 0;
	struct timeval begin,now;
	char chunk[CHUNK_SIZE];
	double timediff;
	
	//make socket non blocking
	fcntl(s,F_SETFL,O_NONBLOCK);

	//beginning time
	gettimeofday(&begin,NULL);

	while(1)
	{
		gettimeofday(&now,NULL);
		
		//TIME ELAPSED IN SECONDS
		timediff = (now.tv_sec - begin.tv_sec)+1e-6*(now.tv_usec-begin.tv_usec);
		
		//if you got some data, then break after timout
		if(total_size > 0 && timediff > timeout)
			break;
		else if(timediff > timeout*2)
			break;
	
		memset(chunk,0,CHUNK_SIZE);
		if((size_recv = recv(s,chunk,CHUNK_SIZE,0)) < 0)
		{
			//if nothing was received wait a little before trying again 0.1 seconds
			usleep(100000);	
		}
		else
		{
			total_size += size_recv;
			printf("%s",chunk);
			//reset beginning time
			gettimeofday(&begin,NULL);
		}
		
	}
	return total_size;
}

int get_line(int sock,char *buffer,int size)
{
	int i= 0;
	char c = '\0';
	int n;
	while((i<size-1)&& (c!='\n'))
	{
		n = recv(sock,&c,1,0);
		if(n > 0)
		{
		   if(c == '\r')
		   {
			n = recv(sock,&c,1, MSG_PEEK);
			if((n>0)&&(c=='\n'))
				recv(sock,&c,1,0);
			else
			  c = '\n';
	           }
 		   buffer[i] = c;
		   i++;		
		}
		else
		   c = '\n';
	}
	buffer[i] = '\0';
	return i;
	
}

std::string getAck(int sockfd){
	std::cout<<"\n receiving ack \n"<<std::endl;
	std::string retStr;
	char buff[24] = {0};
	int n = recv(sockfd,buff,sizeof(buff),0);
	if(n > 0){
		std::string message (buff);
		retStr = message;
	}
	else{
		std::cout<<"error in receiving ack.."<<std::endl;
		retStr = "error in recv";
	}
	return retStr;
}

void sendObject(int sockfd){
	std::cout<<" \n in sendobject function \n"<<std::endl;
	binn *obj;

	// create a new object
	obj = binn_object();

	// add values to it
	char chId[]    = "id";
	char chName[]  = "name";
	char chPrice[] = "price";

	char chNameVal[] = "manish";

	binn_object_set_int32(obj, chId, 1000);
	binn_object_set_str(obj, chName, chNameVal);
	binn_object_set_double(obj, chPrice, 256.55);

	int objsize = binn_size(obj);
	char ibuff[24] = {0};
	snprintf(ibuff,24,"%d",objsize);

	int n = send(sockfd,ibuff,24,0);
	printf("sent to server %d\n",n);

	std::string ack = getAck(sockfd);
	if(ack == "OK"){
			printf("now sending object -- \n");
			// send over the network or save to a file...
			n= send(sockfd, binn_ptr(obj),binn_size(obj),0);
			printf("sent to server %d\n",n);
			ack = getAck(sockfd);
			std::cout<<ack<<std::endl;
	}
	// release the buffer
	binn_free(obj);
}

int main()
{
	int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	char ch = 'A';

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("192.168.0.23");
	address.sin_port = htons(54800);
	len = sizeof(address);
	result = connect(sockfd,(struct sockaddr *)&address,len);

	 print_cpu_time();


	if(result == -1)
	{
		perror("error client connect");
		exit(1);
	}

	char buffer[] = {"manish login exact\r\n"};

	std::string sAck = getAck(sockfd);
	std::cout<<sAck<<std::endl;
	
	while(1)
	{
		//checking if socket is connected
		socklen_t len;
		struct sockaddr_storage addr;
		len = sizeof(addr);
		int nCode = getpeername(sockfd,(struct sockaddr *)&addr,&len);

		printf("\nValue returned by getpeername %d\n",nCode);
		printf("Please enter a command : ");

		//char command[100] ={0};
		std::string sCommand;
		//std::cin>>sCommand;
		std::getline(std::cin,sCommand);
		int n = 0;			
		int length = sCommand.size();
		char ibuff[24] = {0};
		snprintf(ibuff,24,"%d",length);
		n = send(sockfd,ibuff,sizeof(ibuff),0);
		std::string ack =  getAck(sockfd);
		std::cout<<ack<<std::endl;
		if(ack == "OK")
		{
			n = send(sockfd,sCommand.c_str(),sCommand.size(),0);
			ack = getAck(sockfd);
			std::cout<<ack<<std::endl;
			if(sCommand == "object" || sCommand == "OBJECT"){
				sendObject(sockfd);
			}
		}

		printf("sent to server %d\n",n);
		if(n == -1){
			printf("error with server exiting");
			break;
			
		}
		
	}
	close(sockfd);


	return 0;	

}
