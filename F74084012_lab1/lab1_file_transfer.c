#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h> // to get the file size
#include <netinet/in.h> // above are TCP server
#include <netdb.h>  // TCP client
#include <arpa/inet.h> // UDP client
#include <errno.h> // UDP client
#include <time.h>

#define BUFFER_SIZE 256
#define FILE_NAME_MAX_SIZE 50
#define UDP_BUF 256

void error(const char *msg){
	perror(msg);
	exit(1);
} 
int get_file_size(char *filename){
	struct stat statbuf;
	stat(filename,&statbuf);
	int size = statbuf.st_size;
	return size;
}

int main(int argc,char *argv[]){
	

	if (strcmp(argv[1],"tcp") == 0){
		if (strcmp(argv[2],"recv") == 0){
			// do tcp receive file (server)
			int sockfd,newsockfd,portno;
			socklen_t clilen;
			char buffer[256];
			struct sockaddr_in serv_addr, cli_addr;
			struct hostent *server;
			int n;
			if (argc < 5){
				printf("error input");
				exit(1);
			}
			//setting socket type
			sockfd = socket(AF_INET, SOCK_STREAM, 0); // sockfd = socket file descriptor
			if (sockfd < 0 ){
				error("ERROR opening socket");
			}
			bzero((char *) &serv_addr,sizeof(serv_addr));// let n bytes be 0
			portno = atoi(argv[4]); // get port_number
			server = gethostbyname(argv[3]); // get ip (localhost)
			// setting socket address 
			serv_addr.sin_family = AF_INET; // check AF_INET
			bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
			serv_addr.sin_port = htons(portno); // check htons
			// bind let here id bind in the socket
			if(bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))<0)
				error("ERROR on binding");
			// listen : check if any client come? the constant means how many people can get into the server 
			// listen function will create a line which will let the client get into it constant will define how long the line will be
			listen(sockfd,5); // check
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0)
				error("ERROR on accept");
			bzero(buffer,BUFFER_SIZE);
			// get file name
			n = read(newsockfd,buffer,BUFFER_SIZE - 1);
			char FILE_NAME[BUFFER_SIZE];
			strcpy(FILE_NAME,buffer);
			FILE *fp = fopen(buffer,"w");
			if ( fp == NULL){
				error("wrong :opening the file");
			}
			bzero(buffer,BUFFER_SIZE);

			n = write(newsockfd,"i got message",13);

			int length = 0;
			int status = 0;
			time_t now;
			struct tm *p;
			clock_t before_send;
			time(&now);
			p = localtime(&now);
			before_send = clock();
			printf("0%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
			
			//get data until the package is empty
			while(length = read(newsockfd,buffer,BUFFER_SIZE-1)){
				if(length < 0){
					error("receive Data Failed\n");
				}
				//to print the status 25% 50% 75% 100% in details
				if (strcmp(buffer,"next") == 0){
					time(&now);
					p = localtime(&now);
					switch (status){
						case 0:
							printf("25%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
							status++;
							break;
						case 1:
							printf("50%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
							status++;
							break;
						case 2:
							printf("75%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
							status++;
							break;
						case 3:
							printf("100%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
							n= write(newsockfd,"need time",9);
							if (n <0)
								error("errrorrrrr");
							bzero(buffer,BUFFER_SIZE);
							length = read(newsockfd,buffer,BUFFER_SIZE -1 );
							if (length <0)
								error("failed length");
							int another_clock = atoi(buffer);
							clock_t after_send = clock();
							printf("Total trans time: %dms\n" , (int)((after_send - before_send + (clock_t)another_clock)/(double)(CLOCKS_PER_SEC)*1000));
							int FILEsize = get_file_size(FILE_NAME);
							printf("file size :%dMB\n",FILEsize/1000000);
							status++;
							
							break;
					}
					n = write(newsockfd,"i got message",13);
				}
				else{
					// write the data into the file
					int write_length = fwrite(buffer,sizeof(char),length , fp);
					n = write(newsockfd,"i got message",13);
					if (n < 0) 
						error("ERROR: writing to socked");
					if(write_length < length)
					{
						printf("File: write failed\n");
						break;
					}
					bzero(buffer,BUFFER_SIZE);
				}
			}
			fclose(fp);
			close(newsockfd);
			close(sockfd);
					
		}
		else if (strcmp(argv[2],"send") == 0){
			// do tcp send file (client)
			int sockfd,portno,n,length;
			int file_size = 0, sent_size = 0;
			struct sockaddr_in serv_addr;
			struct hostent *server;
			char buffer[BUFFER_SIZE];
			clock_t t1 = clock();
			
			//build up the connection
			if ( argc < 6 ){
				error("error input ");
			}
			
			portno = atoi(argv[4]);
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0)
				error("ERROR , opening socket");
			server = gethostbyname(argv[3]);
			if (server == NULL){
				error("ERROR,no such host");
			}
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr,
				 (char *)&serv_addr.sin_addr.s_addr,
				 server->h_length);
			serv_addr.sin_port = htons(portno);
			if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
				error("ERROR connecting");	
			}
			while(1){

				struct sockaddr_in client_addr;
				socklen_t length = sizeof(client_addr);
				
				// get file name
				char buffer[BUFFER_SIZE];
				bzero(buffer,BUFFER_SIZE);
				strncpy(buffer, argv[5],strlen(argv[5]));
				n = write(sockfd,buffer,strlen(buffer));

				if(length < 0){
					error("recieve data Failed,sending side \n");
				}
				FILE *fp = fopen(buffer,"r");
				file_size = get_file_size(buffer);
				bzero(buffer,BUFFER_SIZE);
				length = read(sockfd,buffer,BUFFER_SIZE - 1);
				if (fp == NULL){
					error("send file not found\n");
				}
				else{
					bzero(buffer,BUFFER_SIZE);
					int file_block_length = 0;
					int status = 0;
					while(!feof(fp)){
						//get the data of the file
						file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE - 1 ,fp);
							if (file_block_length < 0)
								break;
						// send data
						n = write(sockfd,buffer,strlen(buffer));
						sent_size += n; // it is to check how much data it has sent
						bzero(buffer,BUFFER_SIZE);
						//get message
						n = read(sockfd,buffer,BUFFER_SIZE - 1 );
						// to check how much data has been dealt with
						// and print the status and time
						if (sent_size >= (float)file_size*0.25 && status ==0){
							n = write(sockfd,"next",4);
							status++;
							bzero(buffer,BUFFER_SIZE);
							n = read(sockfd,buffer,BUFFER_SIZE - 1);
						}
						else if (sent_size >= (float)file_size * 0.5 && status == 1){
							n = write(sockfd,"next",4);
							status++;
							bzero(buffer,BUFFER_SIZE);
							n = read(sockfd,buffer,BUFFER_SIZE -1 );
						}
						else if (sent_size >= (float)file_size * 0.75 && status == 2){
							n = write(sockfd,"next",4);
							status++;
							bzero(buffer,BUFFER_SIZE);
							n = read(sockfd,buffer,BUFFER_SIZE -1 );
						}
						else if (sent_size >= (float)file_size && status == 3){
							n = write(sockfd,"next",4);
							status++;
							bzero(buffer,BUFFER_SIZE);
							n = read(sockfd,buffer,BUFFER_SIZE - 1);
							if (strcmp(buffer,"need time") == 0){
	                            clock_t t2 = clock();
		                        int cl = t2-t1;
			                    char clocks[30];
				                sprintf(clocks,"%d",cl);
						        n = write(sockfd,clocks,strlen(clocks));
								n = read(sockfd,buffer,BUFFER_SIZE - 1);
							}
						}
						if(n<0){
							break;
						}
						bzero(buffer,BUFFER_SIZE);
					}
					fclose(fp);
					break;

				}
			}
			close(sockfd);
			return 0;
		}
	
	}
	else if (strcmp(argv[1],"udp") == 0){
		if (strcmp(argv[2],"send") == 0){
			// do udp send file 
			if (argc != 6)
				error("udp error input");
			int sock;
			if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
				error("udp socket send");
			//build up the connection
			struct sockaddr_in servaddr;
			memset(&servaddr,0,sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			int portnum = atoi(argv[4]);
			servaddr.sin_port = htons(portnum);
			servaddr.sin_addr.s_addr = inet_addr(argv[3]);

			int ret;
			char sendbuf[UDP_BUF] = {0};
			char recvbuf[UDP_BUF] = {0};
			// open file and send file name
			FILE *fp = fopen(argv[5],"r");
			if(fp == NULL)
				error("open failed");
			strcpy(sendbuf,argv[5]);

			sendto(sock,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
			ret = recvfrom(sock,recvbuf,sizeof(recvbuf), 0, NULL,NULL);
			if (ret == -1)
			{
				if(errno == EINTR){
					printf("errno erro");
				}
				error("recvfrom");
			}
			// record the time
			int file_block_length = 0;
			int sent_file= 0; // check how much data has been sent
			int status = 0;
			int file_size = get_file_size(argv[5]);
			time_t now;
			struct tm *p;
			clock_t before_send;
			time(&now);
			p = localtime(&now);
			before_send = clock();
			printf("0%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
			while(!feof(fp)){
				file_block_length = fread(sendbuf,sizeof(char),UDP_BUF - 1 ,fp);
				sendto(sock,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
				sent_file += strlen(sendbuf);

				if (status == 0 && sent_file >= file_size * 0.25){
					time(&now);
					p = localtime(&now);
					printf("25%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
					status++;
				}
				else if (status == 1 && sent_file >= file_size * 0.5){
					time(&now);
					p = localtime(&now);
					printf("50%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
					status++;
				}
				else if (status == 2 && sent_file >= file_size * 0.75){
					time(&now);
					p = localtime(&now);
                    printf("75%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
                    status++;
                }
				else if (status == 3 && sent_file >= file_size ){
					time(&now);
					p = localtime(&now);
                    printf("100%% %d/%d/%d %d:%d:%d\n",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
                    status++;
					clock_t after_send = clock();
					printf("Total trans time: %dms\n" , (int)(2*((after_send - before_send)/(double)(CLOCKS_PER_SEC)*1000)));
					printf("file size :%d MB\n",file_size/1000000);
                }


				ret = recvfrom(sock,recvbuf,sizeof(recvbuf), 0, NULL,NULL);
	            if (ret == -1)
		        {
			        if(errno == EINTR)
				        continue;
	                error("recvfrom error -> send side");
				}
				else{
//					printf("ret = %d\n",ret);
				}

				memset(sendbuf, 0, sizeof(sendbuf));
				memset(recvbuf, 0, sizeof(recvbuf));
			}
			strcpy(sendbuf,"END!");
			sendto(sock,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
			
			close(sock);
			fclose(fp);
			return 0;
		}
		else if (strcmp(argv[2],"recv") == 0){
			// do udp receive file (server)
			if(argc != 5)
				error("error input");
			// build up the connection
			int sock;
			if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
				error("socket error");
			struct sockaddr_in servaddr;
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			int portnum = atoi(argv[4]);
			servaddr.sin_port = htons(portnum);
			servaddr.sin_addr.s_addr = inet_addr(argv[3]);

			if(bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
				error("bind error:");
			
			
			char recvbuf[UDP_BUF] = {0};

			struct sockaddr_in peeraddr;
			socklen_t peerlen;
			int n;
			peerlen = sizeof(peeraddr);
			memset(recvbuf, 0, sizeof(recvbuf));
			//get filename and open it
			n = recvfrom(sock, recvbuf ,sizeof(recvbuf),0,
						(struct sockaddr *)&peeraddr, &peerlen);
			sendto(sock,recvbuf,n,0,(struct sockaddr *)&peeraddr, peerlen);
			FILE *fp = fopen(recvbuf,"w");
			if(fp == NULL)
				error("open failed");

			while(1){
				//get the data of the file and write into the file
				peerlen = sizeof(peeraddr);
				memset(recvbuf, 0, sizeof(recvbuf));
				n = recvfrom(sock, recvbuf,sizeof(recvbuf),0,
						(struct sockaddr *)&peeraddr, &peerlen);
				if(strcmp(recvbuf,"END!") == 0){
					close(sock);
					break;
				}
				if (n == -1)
				{
					if(errno == EINTR)
						continue;
					error("recvfrom error -> recv side");
				}
				else if (n >0)
				{
					int write_length = fwrite(recvbuf,sizeof(char),n , fp);
					sendto(sock,recvbuf,n,0,(struct sockaddr *)&peeraddr, peerlen);
				}

			}
			return 0;
		}
	}
}

