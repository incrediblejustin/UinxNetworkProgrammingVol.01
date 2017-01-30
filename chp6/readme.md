#client1.cpp & server1.cpp

####1. client1.cpp

1. 用 `shutdown` 来保证当客户端连续发送多个数据分节之后安全关闭。在这种情况下，如果不使用 `shutdown`（使用 `close`），TCP分节不能被完全发送（确认）（数据分节或者确认分节还处在途中）

2. 客户端首先使用 `shutdown`，随即发送一个 FIN 分节来开启正常的4次结束序列，不能直接使用 close，因为close只有在描述符的应用计数为零时才会断开连接

3. 但客户端出现按顺序监视多个描述符时（例如：`read(stdin)->write(serverfd)->read(serverfd)->write(stdout)`），当我们从标准输入中读取时，有可能阻塞在read函数调用上，但是就在这个时候服务器崩溃，客户端将无从得知此消息

4. 利用`select`函数调用可以解决这个问题，`select`允许我们将某些描述符注册在内核中，当这些描述符中有事件（**例如：可读，可写，错误**）发生时，内核将通知应用进程，应用进程检查返回结果来达到见识各个描述符的目的


####2. server1.cpp

1. 实际上，之所以要使用 `signal` 函数调用是因为在 **server1.cpp** 中使用fork函数产生子进程，利用子进程来与客户进行数据交换（关闭 listenfd，并阻塞在 `read` 系统调用上)，同时父进程来继续等待客户与服务器连接（关闭connfd，并阻塞在 `accept` 系统调用上）

2. `signal(SIGCHLD, sig_chld)` 用来**避免僵尸进程**的出现。当连接关闭时，用来服务客户连接的子进程将终止，这时父进程会受到一个 SIGCHLD 信号，如果不捕获并处理该信号，终结的子进程将变成将是进程

3. `waitpid()`函数调用，用来等待子进程关闭. 参数 **WNOHANG** 表示当如果有一个子进程没有终止（当一个进程没有终止，并且不适用该参数时，进程将被阻塞在waitpid函数调用上）我们就回收这个子进程的资源。（这一点是 wait函数调用无法做到的，因为要同时检查多个子进程，并且在子进程未终止时 `wait` 无法通过 **WNOHANG** 参数来避免进程被阻塞）



####3. server2.cpp
1. 使用 `select+for` 来代替 `fork+cli_echo`

2. `allset`是所有描述符的集合, 在每次 for 循环的最开始 rset = allset,  `select` 函数调用将描述符有所改变的结果放在 rset 中，同时返回 nready 表示事件发生的描述符的个数 

3. 在所有事件中，我们只关心两类事件
    - **listenfd 套接字描述符可读**
        
        1. 调用 `accept` 完成取出连接，并返回已连接套接字 connfd
        2. 将 connfd 插入到**client[]**数组中(client是一个整形数组，每一个元素表示一个描述符，非-1的位为等待读的connfd套接字描述符)
        3. 如果有必要的话将 maxfd = connfd, i表示client数组的下标, 如果i 大于 maxi，将 maxi = i。如果 i == FD_SETSIZE,则表示没有更多的文件描述符可用
        4. --nready,如果 nready 大于 0,表示不只有listenfd 可读，转到第二种情况 
        
   - **connfd （在client数组中）套接字描述符可读**
   
	     1. 遍历数组知道下标到达 maxi，小于0表示无可读描述符，否则开始读
        2. 如果读到结果为 0, 关闭描述符，从allset中清理文件描述符，从client中将文件描述符清除，转到第四步
        3. 否则读结果大于 0， 将结果返回给对端
        4. 最重要的已不是利用 --nready 检查是否大于零，如果小于零，提前从for循环中退出


####4. server3.cpp
1. 使用`poll`来代替`select`，`poll`最初只局限于流设备，现在可以工作在热呢描述符上，提供与`select`类似的功能

2. `poll`有一个重要的参数就是`struct pollfd`指针，该指针指向一个`struct pollfd`结构体

```cpp
struct pollfd
{
    int fd;   //需要内核来检查的描述符
    short events;    //对描述符上感兴趣的事件
    short revents;    //函数返回的描述符状态
}
```

