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
        close(STDOUT_FILENO);
        dup(connfd);
        std::cout<<"abcd\n";
        close(connfd);
    }
    close(sockfd);
    return 0;
}

