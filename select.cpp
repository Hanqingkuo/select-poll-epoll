#include<iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc,char * argv[])

{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n",basename(argv[0])) ;
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in addres;
    addres.sin_family = AF_INET;
    addres.sin_port = htons(port);
    inet_pton(AF_INET,ip,&addres.sin_addr);

    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd != -1);

    ret = bind(listenfd,(struct sockaddr*)&addres,sizeof(addres));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);

    struct sockaddr_in cli;
    socklen_t size = sizeof(cli);
    imt connfd = accept(listenfd,(struct sockaddr*)&cli,&size);
    assert(connfd != -1);
    
    char buf[120];
    fd_set read_fds;
    fd_set exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);

    while(1)
    {
        memset(buf,'\0',120);
        FD_SET(connfd,&read_fds);
        FD_SET(connfd,&exception_fds);
        ret = select(connfd+1,&read_fds,NULL,&exception_fds,NULL);
        assert(ret != -1);

        if(FD_ISSET(connfd,&read_fds))
        {
            ret = recv(connfd,buf,119,0);
            if(ret < 0)
            {
                break;
            }
        }
        else if(FD_ISSET(connfd,&exception_fds))
        {
            ret = recv(connfd,buf,119,MSG_OOB);
            if(ret <= 0)
            {
                break;
            }
        }

    }


}


