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
#include <poll.h>
#include <limits.h>
#include "sun.h"

using namespace std;

const int OPEN_MAX = 1024;
const short PORT = 9877;
const int BUFFSIZE = 64;
const int ERROR = -1;
const int INFTIM = 5000;
int main()
{
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen = sizeof(servaddr);
    bzero(&servaddr, sizeof(servaddr));

    int listenfd, connfd;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        err_quit("listen error");
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res;
    if((res = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
    {
        err_quit("bind error");
    }
    if((res = listen(listenfd, 5)) < 0)
    {
        err_quit("listen error");
    }
    
    struct pollfd client[OPEN_MAX];
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(int i = 1; i < OPEN_MAX; ++i)
    {
        client[i].fd = -1;
    }
    int maxi = 0;
    int sockfd, nread, nready;
    char recvbuff[BUFFSIZE];
    for(;;)
    {
        nready = poll(client, maxi + 1, INFTIM);
        if(nready == -1)
        {
            err_quit("select error");
        }
        else if(nready == 0)
        {
            cout << "time out" << endl;
            continue;
        }

        if(client[0].revents & POLLRDNORM)
        {
            if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else
                { 
                    err_quit("accept error");
                }
            }
            int i = 0;
            for(; i < OPEN_MAX; ++i)
            {
                if(client[i].fd < 0)
                {
                    client[i].fd = connfd;
                    client[i].events = POLLRDNORM;
                    break;
                }
            }
            //put new connfd to client[i] less than -1
            if(i == OPEN_MAX)
            {
                err_quit("too many clients");
            }
            if(i > maxi)
            {
                maxi = i;//last index in client
            }
            if(--nready <= 0)
            {
                continue;
            }//continue if no client wirte to server, block on select & wait for it
        }//end of fd_isset(listenfd)
        for(int i = 0; i <= maxi; ++i)
        {
            if((sockfd = client[i].fd) < 0)
            {
                continue;
            }
            if(client[i].revents & POLLRDNORM)
            {
                bzero(recvbuff, BUFFSIZE);
                nread = read(sockfd, recvbuff, BUFFSIZE);
                // read from client, if client shutdown, 
                // 1. close fd
                if(nread == 0)
                {
                    close(sockfd);
                    client[i].fd = -1;
                }
                else if(nread == -1)
                {
                    err_quit("read error");
                }
                else
                {
                    if((res = write(sockfd, recvbuff, nread)) < 0)
                    {
                        err_quit("write error");
                    }
                }

                if(--nready <= 0)
                {
                    break;
                }//break from for, block on select
            }//end of work for client, for

        }
    }

}


