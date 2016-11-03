#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

static const char*status_line[2] = {"200 ok","500 Internet serve error"};

int main(int argc,char *argv[])
{
    if(argc < 4)
    {
        std::cout<<"args : ip_addr port file_name"<<basename(argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *file_name = argv[3];

    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&ser_addr.sin_addr);
    
    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock > 0);

    int ret = -1;
    ret = bind(sock,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
    assert(ret != -1);

    ret = listen(sock,5);
    assert(ret != -1);
    
    struct sockaddr_in cli_addr;
    socklen_t size = sizeof(cli_addr);

    int conned = accept(sock,(struct sockaddr*)&cli_addr,&size);
    if(conned < 0)
    {
        perror("accept error");
    }
    else
    {
        char header_buf[BUFFER_SIZE];
        memset(header_buf,'\0',BUFFER_SIZE);
        char *file_buf;
        struct stat file_stat;
        bool vaild = true;
        int len = 0;
        if(stat(file_name,&file_stat) < 0)
        {
            vaild = false;       
        }
        else
        {
            if(S_ISDIR(file_stat.st_mode))
            {
                vaild = false;
            }
            else if(file_stat.st_mode & S_IROTH)
            {
                int fd = open(file_name,O_RDONLY);
                file_buf = new char[file_stat.st_size + 1];
                memset(file_buf,0,file_stat.st_size + 1);
                if(read(fd,file_buf,file_stat.st_size) < 0)
                {
                    vaild = false;
                }

            }
            else
            {
                vaild = false;
            }
        }

        if(vaild)
        {
            ret = snprintf(header_buf,BUFFER_SIZE-1,"%s %s\r\n","HTTP/1.1",status_line[0]);
            len += ret;
            ret = snprintf(header_buf+len,BUFFER_SIZE-len-1,"Content-Length: %d\r\n",file_stat.st_size);
            len += ret;
            ret = snprintf(header_buf+len,BUFFER_SIZE-len-1,"%s","\r\n");

            struct iovec iv[2];
            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);
            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;
            ret = writev(conned,iv,2);
        }
        else
        {
            ret = snprintf(header_buf,BUFFER_SIZE-1,"%s %s\r\n","HTTP/1,1",status_line[1]);
            len += ret;
            ret = snprintf(header_buf+len,BUFFER_SIZE-1-len,"%s","\r\n");
            send(conned,header_buf,strlen(header_buf),0);
        }
        close(conned);
        delete file_buf; 
    }
    close(sock);
    return 0;
}


