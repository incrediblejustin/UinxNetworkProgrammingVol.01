#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "sun.h"
using namespace std;


const short PORT = 9877;
const int BUFFSIZE = 64;
const int ERROR = -1;



void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	if((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{
		printf("child %d terminated\n", pid);
	}
	return;
}

void str_echo(int connfd)
{
	ssize_t n;
	char buf[BUFFSIZE];
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	bzero(&cliaddr, clilen);
	if(getpeername(connfd,(struct sockaddr*)&cliaddr, &clilen) < 0)
	{
		cout << "getpeername of client error: " << errno << endl;
		return;
	}
	while(1)
	{
		if((n = read(connfd, buf, BUFFSIZE)) <= 0)
		{
			if(n == 0)
			{
				cout << "client " << ntohs(cliaddr.sin_port) << "(port) shutdown" << endl;
			}
			else if(n < 0)
			{
				cout << "read error(line 55): " << errno << endl;
			}
			return;
		}	
		write(connfd, buf, n);
	}
	
}
int main()
{
    pid_t childpid;
	int listenfd, connfd;
	struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);	
    bzero(&cliaddr, sizeof(cliaddr));	
    
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error: " << errno << endl;
		exit(ERROR);
	}
    
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);
	servaddr.sin_addr.s_addr   = htonl(INADDR_ANY);

    const int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        err_quit("setsockopt error");
    }
	int res = 0;
	if((res = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
	{
	    err_quit("bind error");
    }
    if(listen(listenfd, 5) < 0)
    {
        err_quit("listen error");
    }




    int udpfd;
	if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		cout << "socket error: " << errno << endl;
		exit(ERROR);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);
	servaddr.sin_addr.s_addr   = htonl(INADDR_ANY);

	if((res = bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
	{
        err_quit("bind error");
	}
	
   
    
    Signal(SIGCHLD, sig_chld);
    fd_set rset;
    FD_ZERO(&rset);
    int MAXFDP1 = listenfd > udpfd ? listenfd + 1 : udpfd + 1;
    int nready;
    int nread;
    char msg[BUFFSIZE];

    
    
    for(;;)
	{
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if((nready = select(MAXFDP1, &rset, NULL, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                err_quit("select error");
            }
        }
        
        if(FD_ISSET(listenfd, &rset))
        {
            
		    if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
		    {
			    if(errno == EINTR)
				    continue;
		    	else
			    {
				    err_quit("accept error");
			    }      
		    }
            
		    if((childpid = fork()) == 0)
		    {
		    	close(listenfd);
		    	str_echo(connfd);
		    	exit(0);
		    }   	
		    close(connfd);
            
        }
        if(FD_ISSET(udpfd, &rset))
        {
            nread = recvfrom(udpfd, msg, BUFFSIZE, 0, (struct sockaddr*)&cliaddr, &clilen);
            sendto(udpfd, msg, nread, 0, (struct sockaddr*)&cliaddr, clilen);

        }

	}

    exit(0);
}
