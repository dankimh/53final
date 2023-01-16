#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#define MAX 1024 //max client number(=FD_SETSIZE)
#define BUF_LEN 1024 //max buffer length
int main(int argc, char *argv[])
{
	//argv error handling
	if(argc!=2){
		printf("usage : %s <port> \n" , argv[0]);
    	exit(1);
    }
    
	//create IPv4, TCP socket
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1) { //socket create error handler
		printf("create socket fail\n");
		return -1;
	}
	else {
		printf("creat socket success , sockfd = %d\n",sockfd);
	}
	//initialize server address information
	struct sockaddr_in seraddr,cliaddr;
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(atoi(argv[1]));
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(seraddr.sin_zero,0,8);
    
	//allocate server address
	socklen_t len = sizeof(struct sockaddr);
	int bindret = bind(sockfd,(struct sockaddr *)&seraddr,len);
	if(bindret == -1) { //bind error handler
		printf("bind fail\n");
		close(sockfd);
		return -2;
	}
	else {
		printf("bind success\n");
	}
    
	//listen for client(max number 1024-FD_SETSIZE)
	int listenret = listen(sockfd,MAX);
	if(listenret == -1) { //listen error handler
		printf("listen fail\n");
		close(sockfd);
		return -3;
	}
	else {
		printf("listen success\n");
	}
	//fd_set, current max file descriptor
	fd_set read,readset;
	int maxfd = sockfd;
	//initialize readset&set sockfd to 1
	FD_ZERO(&readset);
	FD_SET(sockfd,&readset);
	while(1) {
		//clientfd
		int confd;
		read = readset;
		//select which file descriptor has changed
		int selectret = select(maxfd+1,&read,NULL,NULL,NULL);
		if(selectret == -1) { //select error handler
			printf("select fail\n");
			close(sockfd);
			return -4;
		}
        
		//if socket for tcp request has changed
		if(FD_ISSET(sockfd,&read)) {
			//accept client&allocate confd to client
			confd = accept(sockfd,(struct sockaddr *)&cliaddr,&len);
			printf("connect client(%s) success\nconfd = %d\n",inet_ntoa(cliaddr.sin_addr),confd);
			//update maxfd
			if(maxfd < confd) {
				maxfd = confd;
			}
			//set confd in readset
			FD_SET(confd,&readset);
        
			//if sockfd is the only file descriptor changed
			if(selectret == 1) {
				continue;
			}
		}
        
		//message from client
		char buf[BUF_LEN];
		int i;
		
		//for the rest of file descriptors(client's file descriptor)
		for(i = sockfd+1;i<=maxfd;i++) {
			memset(buf,0,BUF_LEN);
			
			//if client's file descriptor has changed
			if(FD_ISSET(i,&read)) {
				if(recv(i,buf,BUF_LEN,0) == 0) { //if end of data
					FD_CLR(i,&readset); //clear file descriptor
					printf("----------client(%s) quit---------\n",inet_ntoa(cliaddr.sin_addr));
					continue;
				}
				else {
					printf("client(%d) send message\n",i);
					send(i,buf,BUF_LEN,MSG_CONFIRM); //echo buf to client
					if(strncmp(buf,"end",3) == 0) { //end of connection
						FD_CLR(i,&readset); //clear file descriptor
						printf("----------client(%s) quit---------\n",inet_ntoa(cliaddr.sin_addr));
						continue;
					}
				}
			}
		}
	}
	return 0;
}
