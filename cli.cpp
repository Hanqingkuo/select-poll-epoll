#include<iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <event.h>
#include <event2/util.h>

int tcp_connnect_serve(const char *server_ip,int port);
void cmd_msg_cb(int fd,short events,void *arg);
void socket_read_cb(int fd,short events,void *arg);

int main(int argc,char**argv)
{
    if(argc < 3)
    {
        printf("please input 2\n");
        return -1;
    }

    int sockfd = tcp_connnect_serve(argv[1],atoi(argv[2]));
    if(sockfd == -1)
    {
        perror("tcp_connnect_serve error");
        return -1;
    }
    printf("connect success\n");
    struct event_base *base = event_base_new();
    struct event *ev_sockfd = event_new(base,sockfd, EV_READ | EV_PERSIST,sockfd_read_cb,NULL);
}

