#include<stdio.h>
#include<signal.h>
#include<unistd.h>

int g_count=20;
void sigcallback(int signumber)
{
    printf("i recv signumber is %d\n",signumber);
    g_count--;
}
int main()
{
    signal(2,sigcallback);
    int i=1;
    while(g_count--)
    {
        printf("%d\n",i);
        i++;
        sleep(1);
    }
    while(1)
    {
        printf("i am main thread\n");
        sleep(1);
    }
    return 0;
}
