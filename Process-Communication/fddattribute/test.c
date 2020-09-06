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
    int flagread=fcntl(fd[0],F_GETFL);
    printf("flagread=%d\n",flagread);
    int flagwrite=fcntl(fd[1],F_GETFL);
    printf("flagwrite=%d\n",flagwrite);
    flagwrite |=O_NONBLOCK;
    printf("O_NONBLOCK flag=%d\n",flagwrite);
    fcntl(fd[1],F_SETFL,flagwrite);
    return 0;
}
