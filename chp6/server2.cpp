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
    
    //initialize , max fd, index into client, the elementory of client
    //-1 indicates avialable entry
    int maxfd, maxi, nready, client[FD_SETSIZE];
    maxfd = listenfd;
    maxi = -1;
    for(int i = 0; i < FD_SETSIZE; ++i)
    {
        client[i] = -1;
    }

    //set allset to zero
    //set listenfd to allset
    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    
    //read from this fd
    int sockfd;
    char recvbuff[BUFFSIZE];
    int nread;
    for(;;)
    {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(nready == -1)
        {
            err_quit("select error");
        }
        else if(nready == 0)
        {
            cout << "time out" << endl;
            continue;
        }

        if(FD_ISSET(listenfd, &rset))
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
            //put new connfd to client[i] less than -1
            int i = 0;
            for(; i < FD_SETSIZE; ++i)
            {
                if(client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE)
            {
                err_quit("too many clients");
            }
            FD_SET(connfd, &allset);
            if(connfd > maxfd)
            {
                maxfd = connfd;//maxfd should be the biggst, also used for select maxfd+1
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
            if((sockfd = client[i]) < 0)
            {
                continue;
            }
            if(FD_ISSET(sockfd, &rset))
            {
                bzero(recvbuff, BUFFSIZE);
                nread = read(sockfd, recvbuff, BUFFSIZE);
                // read from client, if client shutdown, 
                // 1. close fd
                // 2. clear fd in the allset
                // 3. clear fd in the client
                if(nread == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
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


