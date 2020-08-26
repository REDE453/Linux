#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

int main()
{
    int pid=fork();
    if(pid<0)
    {
        perror("fork");
        return 0;
    }
    else if(pid==0)
    {
        //child
        printf("i am child\n");
        sleep(5);
        exit(15);
    }
    else
    {
        //father
        int status;
        while(waitpid(pid,&status,WNOHANG)==0);

        if((status&0x7F)==0)
        {
            printf("正常退出\n");
            printf("exitcode=%d\n",(status>>8)& 0xFF);
        }
        else
        {
            printf("异常退出\n");
        }
        while(1)
        {
            sleep(3);
            printf("i am father ,i don't have time\n");
        }
    }
    return 0;
}
