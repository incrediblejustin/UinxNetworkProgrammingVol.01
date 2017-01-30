#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include "sun.h"
using namespace std;


const int ERROR = -1;
const short SERV_PORT = 9877;
const int BUFFSIZE = 128;

void dg_cli(FILE* fp, int sockfd, struct sockaddr* servaddr, socklen_t servlen)
{
    int n;
    char sendline[BUFFSIZE], recvline[BUFFSIZE + 1];

    socklen_t len;
    struct sockaddr *replyaddr;
    replyaddr = (struct sockaddr*)malloc(servlen);
    if(replyaddr == NULL)
    {
        err_quit("malloc error");
    }
    while(Fgets(sendline, BUFFSIZE, fp) != NULL)
    {
        n = sendto(sockfd, sendline, BUFFSIZE, 0, servaddr, servlen);
        if(n < 0)
        {
            err_quit("sendto error");
        }
        n = recvfrom(sockfd, recvline, BUFFSIZE, 0, replyaddr, &len);
        if(len != servlen || memcmp(servaddr, replyaddr, len) != 0)
        {
            cout << "reply from " << sock_ntop(replyaddr, len) << endl;
            continue;
        }
        recvline[n] = '\0';
        cout << recvline;
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    dg_cli(stdin, sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    exit(0);
}
