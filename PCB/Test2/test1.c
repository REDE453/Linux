#include<stdio.h>
#include<unistd.h>

int main()
{
    int a=10;
    int b=20;
    int c=a+b;
    printf("c=%d\n",c);
    int pid=fork();
    if(pid<0)
    {
        perror("fork");
        return 0;
    }
    else if(0==pid)
    {
        //子进程
        printf("i am child %d\n",getpid());
    }
    else
    {
        printf("i am father %d\n",getpid());
        //父进程
        while(1)
        {

        }
    }
    while(1)
    {
        sleep(3);
    }
    return 0;
}
