/* Receiver/client multicast Datagram example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 1024

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int datalen;
char databuf[BUFFER_SIZE];

int get_file_size(char *filename){
	struct stat statbuf;
	stat(filename,&statbuf);
	int size = statbuf.st_size;
	return size;
}

int main(int argc, char *argv[])
{
/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else
	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else
		printf("Setting SO_REUSEADDR...OK.\n");
	
	 
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(4321);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		printf("Binding datagram socket...OK.\n");
	 
	/* Join the multicast group 226.1.1.1 on the local address*/
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
	/* your ip address */ 
	group.imr_interface.s_addr = inet_addr("10.0.2.15"); 
	/* IP_ADD_MEMBERSHIP:  Joins the multicast group specified */ 
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		printf("Adding multicast group...OK.\n");
	 
	/* Read from the socket. */
	datalen = sizeof(databuf);
	
	if(recvfrom(sd, databuf, datalen, 0, NULL, NULL) < 0)
	{
		perror("Reading datagram message error");
		close(sd);
		exit(1);
	}
	FILE *fp = fopen(databuf,"w");
	char file_name[BUFFER_SIZE];
	strcpy(file_name,databuf);
	while(1){
		bzero(databuf,BUFFER_SIZE);
		if(recvfrom(sd,databuf,datalen,0,NULL,NULL) < 0)
		{
			perror("reading file message error");
			close(sd);
			exit(1);
		}
		if(strcmp(databuf,"finish") == 0){
			int size = get_file_size(file_name);
			printf("reading datagram message...OK\n");
			printf("receive file size: %d KB\n",size/1000);
			return 0;
		}

		fwrite(databuf,sizeof(char),sizeof(databuf),fp);
	}
	return 0;
}
