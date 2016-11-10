#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];

int setnonblocking(int fd)
{
    int old_op = fcntl(fd,F_GETFL);
    int new_op = old_op | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_op);
    return old_op;
}

void addfd(int epollfd,int fd)
{
    struct epoll_event events;
    events.data.fd = fd;
    events.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&events);
    setnonblocking(fd);
}

void sig_hanldler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}
void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_hanldler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("usage : %s ip port\n",basename(argv[0]));
        exit(0);
    }
    const char *ip = argv[0];
    int port = atoi(argv[1]);

    int ret =  -1;
    struct sockaddr_in addres;
    bzero(&addres,sizeof(addres));
    addres.sin_family = AF_INET;
    addres.sin_port = htons(port);
    inet_pton(AF_INET,ip,&addres.sin_addr);

    int listened = socket(AF_INET,SOCK_STREAM,0);
    assert(listened >= 0);

    ret = bind(listened,(struct sockaddr*)&addres,sizeof(addres));
    if(ret == -1)
    {
        printf("errno is %d\n",errno);
        return 1;
    }
    ret = listen(listened,5);
    assert(ret != -1);
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listened);

    ret = socketpair(AF_UNIX,SOCK_STREAM,0,pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd,pipefd[0]);

    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);
    bool serve_stop = false;

    while(!serve_stop)
    {
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if((number < 0) && (errno = EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listened)
            {
                struct sockaddr_in cliaddr;
                socklen_t size = sizeof(cliaddr);
                int connfd = accept(sockfd,(struct sockaddr*)&cliaddr,&size);
                addfd(epollfd,connfd);
            }
            else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                int ret = recv(sockfd,signals,sizeof(signals),0);
                if(ret <= 0)
                {
                    continue;
                }
                else
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {
                            case SIGCHLD:
                            case SIGHUP: 
                            {
                                continue;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                serve_stop = true;
                                break;
                            }

                        }
                    }
                }
            }
            else
            {
                
            }
        }
    }
    printf("close fds\n");
    close(listened);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;

}
