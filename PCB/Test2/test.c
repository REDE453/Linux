#include<stdio.h>
#include<unistd.h>
int main()
{
    printf("begin...\n");
    fork();
    int pid=getpid();
    printf("end...pid=%d\n",pid);
    return 0;
}
