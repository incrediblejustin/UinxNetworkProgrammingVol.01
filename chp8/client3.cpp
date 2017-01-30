#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "sun.h"
using namespace std;

const int NDG = 2000;
const int DGLEN = 1400;
const int ERROR = -1;
const short SERV_PORT = 9877;
const int BUFFSIZE = 128;

void dg_cli(FILE* fp, int sockfd, struct sockaddr* servaddr, socklen_t servlen)
{
    int n;
    char sendline[BUFFSIZE], recvline[BUFFSIZE + 1];
    for(int i = 0; i < NDG; ++i)
    {
        sendto(sockfd, sendline, DGLEN, 0, servaddr, servlen);
    }
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cout << "usage: a.out <ip address>" << endl;
        exit(ERROR);
    }
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        err_quit("socket error");
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port  = htons(SERV_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        err_quit("inet_pton");
    }
    dg_cli(stdin, sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    exit(0);
}
