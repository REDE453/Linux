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
    int fd=open("./fifo",O_WRONLY);
    if(fd<0)
    {
        perror("open");
        return 0;
    }
    write(fd,"nice day",8);
    close(fd);
    return 0;
}
