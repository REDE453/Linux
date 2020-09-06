#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
    int fd=open("./tmpfile",O_RDWR|O_CREAT,0064);
    if(fd<0)
    {
        perror("open");
        return 0;
    }
    printf("fd=%d\n",fd);
    //将标准输出重定向到tmpfile中
    dup2(3,1);
    printf("hehe\n");

    return 0;
}
