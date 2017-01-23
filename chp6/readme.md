##client1.cpp & server1.cpp

####client1.cpp
1. use `shutdown` to guarantee when client input so many data, then shut down is safe,  in this situation , if we don't use`shutdown`, tcp segments can't be send or comfirm completely

at this time, client may use 'shutdown()' first, start to send a FIN segment(*can not use 'close()', because close only send FIN when the descipter's reference number become zero) 

2. sometimes this situation will happen, order: read(stdin)->write(serverfd)->read(serverfd)->write(stdout),  when we read stdin, wemay block right here, but if server crush right now, we may not know it, because stdin are waiting for some input.

that why we need to use 'select()', select let us register some decripter and the type when thing happened (like input, output, error) to kernel, if those thing dose happen, kernel will tell us(user process) right way

####server1.cpp
1. `signal(SIGCHLD, sig_chld)` are used to kill process, when client shutdown, and process terminated. user process will catch a SIGCHLD signal

 waitpid(), wait for the child process close, we use WHOHANG, so we will not block when the child process are not close, now we can use while check all child process(this is why we don't use `wait`, yes WNOHANG is good solution with waitpid)


2. actually, the reason why we use 'signal' is when 'accept()' returns the value  greater than zero, we will use `fork()` to get a new child process to handle client link, is this way , we will close listenfd in child process, and str_echo(connfd) will do all the work with client, then exit;  parent process should close connfd right away, then for loop will let parent process block on accept

####server2.cpp
1. use select+for instead of fork+cli_echo

2. set every fd into `allset`, rset = allset in the begining of every for loop, then `select` will change every bit of rset,
and returns the value nready, stands of the number of ready to read

3. two thing we should care about
    1. listenfd is readable
        1. accept from it, returns the value connfd
        2. set connfd into client(this is a intger array, each elementory stand a fd whitch waiting for read)
        3. maxfd = connfd if it is necessary, maxi = i(the biggst index that have been insert of client for now, if i == FD_SETSIZE, that means no more fd for this process)
        4. --nready, then if nready still bigger than 0, it means not only listenfd readable, but also connfd readable, that leads us to the second situation
    2. connfd in the client[] are readable
        1. for-loop, if some fd are readable(`client[i] > 0`), untill i bigger than maxi
        2. read from client[i], if returns 0, close fd, clear fd in the allset, clear fd in the client, then break
        3. or, if returns the value bigger the 0, send it ro client
        4. most important, check nready after send massage, it will save a lot of time
