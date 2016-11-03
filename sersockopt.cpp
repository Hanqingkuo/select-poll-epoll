#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        std::cout<<"args:ip_addr port_addr recv_buf_size:"<<basename(argv[0]);
        return 1;
    }
    
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in seraddr,cliaddr;
    bzero(&seraddr,sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&seraddr.sin_addr);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd > 0);

    int recvbuf = atoi(argv[3]);
    socklen_t size = sizeof(recvbuf);
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&recvbuf,size);
    getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&recvbuf,&size);
    std::cout<<"seting recvbuf is"<<recvbuf<<"\n";

    int ret = -1;
    ret = bind(sockfd,(struct sockaddr*)&seraddr,sizeof(seraddr));
    assert(ret !=  -1);

    ret = listen(sockfd,5);
    assert(ret != -1);
    socklen_t cli_size = sizeof(cliaddr);
    int connfd = accept(sockfd,(struct sockaddr*)&cliaddr,&cli_size);
    if(connfd < 0)
    {
        perror("accept error");
    }
    else
    {
        char buf[BUF_SIZE];
        memset(buf,'\0',BUF_SIZE);
        while(recv(connfd,buf,BUF_SIZE-1,0) > 0)
        {}
        close(connfd);

    }
    close(sockfd);
    return 0;
}

