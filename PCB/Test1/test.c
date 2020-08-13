#include<stdio.h>
#include"test.h"
#include<unistd.h>
int main()
{
    while(1)
    {
        sleep(3);
        func();
    }
    return 0;
}
