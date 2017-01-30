#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <stdlib.h>
#include <strings.h>
#include "sun.h"
using namespace std;

const short SERV_PORT = 9877;
const int BUFFSIZE    = 128;
const int ERROR       = -1;

static int count;
static void recvfrom_int(int signo)
{
    cout << endl;
    cout << "received " << count << "datagrams" << endl;
    exit(0);
}
void dg_echo(int sockfd, struct sockaddr* cliaddr, socklen_t clilen)
{
    int n;
    char msg[BUFFSIZE];
    socklen_t len;
    signal(SIGINT, recvfrom_int);
    
    n = 220 * 1024;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) < 0)
    {
        err_quit("setsockopt error");
    }
    for(;;)
    {
        len = clilen;
        recvfrom(sockfd, msg, BUFFSIZE, 0, cliaddr, &len);

        count++;
    }
}

int main()
{
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        err_quit("socket error");
    }

    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        err_quit("bind error");
    }

    dg_echo(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    exit(0);
}

