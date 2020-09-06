#include<stdio.h>
#include<unistd.h>
#include<signal.h>

void sigcallback(int signum)
{
    printf("i am SIGINT:%d\n",signum);
    return;
}
int main()
{
    signal(SIGINT,sigcallback);
    signal(SIGQUIT,sigcallback);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
