#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>

int main()
{
    int fd[2];
    int ret=pipe(fd);
    if(ret<0)
    {
        perror("pipe");
        return 0;
    }
    int pid=fork();
    if(pid<0)
    {
        perror("pid");
        return 0;
    }
    else if(pid==0)
    {
        //child;
        close(fd[0]);
        int flag=fcntl(fd[1],F_GETFL);
        flag|=O_NONBLOCK;
        fcntl(fd[1],F_SETFL,flag);
        printf("i am child process,begin write\n");
        write(fd[1],"123",3);

    }
    else
    {
        //father
        close(fd[0]);
        //进程等待
        int status;
        wait(&status);
        printf("signal num is %d\n",status & 0x7f);
    }
    return 0;
}
