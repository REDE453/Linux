#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void func()
{
    printf("还会调用我\n");
}
int main()
{
    atexit(func);
    printf("begin~~\n");
    _exit(1);
    printf("end~~\n");
    return 0;
}
