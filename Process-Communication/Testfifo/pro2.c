#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
    //int ret=mkfifo("./fifo",0664);
    //if(ret<0)
    //{
    //    perror("fifo");
    //    return 0;
    //}
    int fd=open("./fifo",O_RDONLY,0064);
    if(fd<0)
    {
        perror("open");
        return 0;
    }
    char buf[1024]={0};
    read(fd,buf,sizeof(buf)-1);
    printf("%s\n",buf);
    close(fd);
    return 0;
}
