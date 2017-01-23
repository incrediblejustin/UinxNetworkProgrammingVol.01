#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <strings.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
using namespace std;


const short PORT = 9877;
const int BUFFSIZE = 64;
const int ERROR = -1;
void str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[BUFFSIZE];
	int		n;
	int res;
	stdineof = 0;
	FD_ZERO(&rset);
	for ( ; ; )
   	{
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = (fileno(fp) > sockfd ? fileno(fp) : sockfd) + 1;
		if((res = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
		{
			cout << "select error: " << errno << endl;
		}

		if (FD_ISSET(sockfd, &rset)) 
		{	/* socket is readable */
			if ( (n = read(sockfd, buf, BUFFSIZE)) == 0) 
			{
				if (stdineof == 1)
					return;		/* normal termination */
				else
				{
					cout << "str_cli error: server terminated prematurely" << endl;
					return;
				}
			}

			write(fileno(stdout), buf, n);
		}

		if (FD_ISSET(fileno(fp), &rset))
	   	{  /* input is readable */
			if ( (n = read(fileno(fp), buf, BUFFSIZE)) == 0)
		   	{
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);	/* send FIN */
				FD_CLR(fileno(fp), &rset);
				continue;
			}

			write(sockfd, buf, n);
		}
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		cout << "usage: a.out <IpAddress>" << endl;
	    exit(ERROR);
	}

	int sockfd, n;
	struct sockaddr_in servaddr;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error: " << errno << endl;
		exit(ERROR);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);
	int res = 0;
	if((res = inet_pton(AF_INET, argv[1], &servaddr.sin_addr)) <= 0)
	{
		cout << "inet_pton error: " << errno << endl;
		exit(ERROR);
	}
	
	if((res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
	{
		cout << "connect error: " << errno << endl;
		exit(ERROR);
	}
	
	str_cli(stdin, sockfd);
    exit(0);
}
