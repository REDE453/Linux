#include<stdio.h>
#include<unistd.h>

int main()
{
    int fd[2];
    int ret=pipe(fd);
    if(ret<0)
    {
        perror("pipe");
        return 0;
    }
    printf("fd[0]=%d\n",fd[0]);
    printf("fd[1]=%d\n",fd[1]);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
