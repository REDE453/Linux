#include<stdio.h>
#include<unistd.h>
#include<signal.h>

int main()
{
    printf("i am stop!\n");
    //int* p=NULL;
    //*p=10;
    //kill(getpid(),2);
    alarm(3);
    while(1)
    {
        printf("程序有问题！\n");
        sleep(1);
    }
    return 0;
}
