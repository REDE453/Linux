#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>

int main()
{
    //int mkfifo(const char* pathname,mode_t mode);
    mkfifo("./fifo1",0664);
    return 0;
}
