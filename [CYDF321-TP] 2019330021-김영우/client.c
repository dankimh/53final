#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>

#define BUF_LEN 1024 // max buffer length

int main(int argc, char *argv[])
{
	//argv error handling
	if (argc != 3) {
        	printf("usage : %s <ip> <port> \n" , argv[0]);
        	exit(1);
    	}
    	
    	//create IPv4, TCP socket
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0) { // socket creation error handler
		printf("creat socket fail\n");
		return -1;
	}
	else {
		printf("creat socket success , sockfd = %d\n",sockfd);
	}

	//initialize server address information
	struct sockaddr_in cliaddr;
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(atoi(argv[2]));
	cliaddr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(cliaddr.sin_zero,0,8);

	//connect request
	int confd = connect(sockfd,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr));
	if(confd < 0) { //connection error handler
		printf("connect fail\n");
		close(sockfd);
		return -2;
	}
	else {
		printf("----------connect success-----------\nconfd = %d\n",confd);
	}
	
	// message buffer
	char buf[BUF_LEN];
	int str_len,input_len;
    
    	//fd_set&initialize readset&add stdin/sockfd in readset
	fd_set reads, readset;
	FD_ZERO(&readset);
	FD_SET(sockfd,&readset);
	FD_SET(0,&readset);
    
	while(1) {
		reads=readset;
		
		//select which file descriptor has changed
		if(select(sockfd+1,&reads,NULL,NULL,NULL)==-1) { //select error handler
			printf("select() : error\n");
			exit(-1);
		}
    	
    		//if stdin has changed
		if(FD_ISSET(0,&reads)) {
			memset(buf,0,BUF_LEN);
			input_len=read(0,buf,BUF_LEN); //read buffer from stdin
			send(sockfd,buf,strlen(buf),0); //send it to the server
		}
		
		//if sockfd has changed(recv from server)
		if(FD_ISSET(sockfd,&reads)) {
			if(str_len=recv(sockfd,buf,BUF_LEN,0)) { //received buffer from server
				buf[str_len]='\0';
				printf("echo - ");
				fputs(buf, stdout); // echo it
			}
			else { // no response from server
				FD_CLR(sockfd,&readset); //clear sockfd from readset
				printf("server closed\n");
				close(sockfd);
				break;
			}
		} 
	}
	close(sockfd);
	return 0;
}
