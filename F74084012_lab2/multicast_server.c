/* Send Multicast Datagram code example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
#define BUFFER_SIZE 1024
char databuf[BUFFER_SIZE];
int datalen = sizeof(databuf);
 
int get_file_size(char *filename){
	struct stat statbuf;
	stat(filename,&statbuf);
	int size = statbuf.st_size;
	return size;
}

void error(const char *msg){
	perror(msg);
	exit(1);
}

int main (int argc, char *argv[ ])
{
/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  printf("Opening the datagram socket...OK.\n");
	 
	/* Initialize the group sockaddr structure with a */
	/* group address of 226.1.1.1 and port 4321. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(4321);
	 
	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr("10.0.2.15");
	// 192.168.32.143
	
	/* IP_MULTICAST_IF:  Sets the interface over which outgoing multicast datagrams are sent. */
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
	  perror("Setting local interface error");
	  exit(1);
	}
	else
	  printf("Setting the local imulticast interface...OK\n");
	/* Send a message to the multicast group specified by the*/
	/* groupSock sockaddr structure. */
	/*int datalen = 1024;*/
	
	FILE *fp = fopen(argv[1],"r");
	if (fp == NULL){
		error("fp error");
	}
	strcpy(databuf,argv[1]);
	//databuf = argv[1];
	datalen = sizeof(databuf);
	if(sendto(sd,databuf,datalen,0,(struct sockaddr*)&groupSock, sizeof(groupSock))<0)
	{
		perror("sending filename error");
	}
	while(!feof(fp)){
		bzero(databuf,BUFFER_SIZE);
		int file_block_length = fread(databuf,sizeof(char),BUFFER_SIZE - 1,fp);
		if (file_block_length < 0)
			break;
		datalen = sizeof(databuf);
		sendto(sd,databuf,datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
		
	
	}

	bzero(databuf, BUFFER_SIZE);
	strcpy(databuf,"finish");
	datalen = sizeof(databuf);
	sendto(sd, databuf,datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));

	int size = get_file_size(argv[1]);
	printf("sending datagram message...OK\n");
	printf("file size : %dKB\n",size/1000);

	return 0;
}
