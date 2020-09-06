#include<stdio.h>
#include<unistd.h>
#include<signal.h>

struct sigaction oldact;
void sigcallback(int signum)
{
    printf("signum:%d\n",signum);
    sigaction(2,&oldact,NULL);
}
int main()
{
    struct sigaction newact;
    newact.sa_handler=sigcallback;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags=0;
    sigaction(2,&newact,&oldact);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
