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
    int flagwrite=fcntl(fd[0],F_GETFL);
    flagwrite |=O_NONBLOCK;
    fcntl(fd[0],F_SETFL,flagwrite);
    close(fd[1]);
    char buf[1024]={0};
    int readsize=read(fd[0],buf,sizeof(buf)-1);
    printf("readsize=%d\n",readsize);
    perror("read");
    return 0;
}
