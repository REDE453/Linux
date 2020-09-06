#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
    int fd[2];
    int ret=pipe(fd);
    if(ret<0)
    {
        perror("pipe");
        return 0;
    }
    int flagwrite=fcntl(fd[1],F_GETFL);
    flagwrite |=O_NONBLOCK;
    fcntl(fd[1],F_SETFL,flagwrite);
    close(fd[0]);
    int count=0;
    while(1)
    {
        printf("begin write\n");
        int writesize=write(fd[1],"1",1);
        if(writesize<0)
        {
           printf("writesize=%d\n",writesize);
           perror("write");
           break;
        }
        count++;
    }
    printf("count=%d\n",count);
    return 0;
}
