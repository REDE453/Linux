#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

void swap(int* a,int* b)
{
    int c=*a;
    *a=*b;
   * b=c;
}
int main()
{
    int a=10;
    int b=20;
    swap(&a,&b);
    printf("a=%d b=%d\n",a,b);
    printf("hahaha");
    fflush(stdout);
    _exit(1);
    return 0;
}
