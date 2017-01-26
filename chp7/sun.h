#ifndef _SUN_H_
#define _SUN_H_

#include <signal.h>
#include <error.h>
#include <iostream>
using namespace std;
typedef void Sigfunc(int);

/*
Sigfunc * Signal(int signo, Sigfunc *func)
{
	struct signaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.as_mask);
	act.sa_flags = 0;
	if(signo == SIGALRM)
	{
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else
	{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}

	if(signaction(signo, &act, &oact) < 0)
	{
		return SIG_ERR;
	}
	return oact.sa_handler;
}

*/


Sigfunc * Signal(int signo, Sigfunc *func)
{
	Sigfunc *sigfunc;
	if((sigfunc = signal(signo, func)) == SIG_ERR)
	{
		cout << "signal error: " << errno << endl;
	}
	return sigfunc;
}

void err_quit(const char * error)
{
    cout << error << ": " << errno << ',' << strerror(errno) << endl;
    exit(1);
}

int setnonblocking(int fd)
{
    int oldflag;
    if((oldflag = fcntl(fd, F_GETFL,0)) < 0)
    {
        err_quit("fcntl getflag error");
    }
    int newflag = old | O_NONBLOCK;
    if(fcntl(fd, F_SETFL, newflag) < 0)
    {
        err_quit("fcntl setflag NONBLOCK error");
    }
    return oldflag;
}
#endif
