#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

//获取环境变量
//1.main函数的参数获取环境变量
#if 0
int main(int argc,char* argv[],char* env[])
{
    //argc-->命令行参数个数
    //argv-->命令行参数具体的内容
    //env-->环境变量
    for(int i=0;i<argc;i++)
    {
        printf("%s\n",argv[i]);
    }
    //char* env[]
    //env[0]=xxx
    //env[1]=yyy
    //env[2]=xxx
    //env[3]=NULL
    for(int i=0;env[i];i++)
    {
        printf("%s\n",env[i]);
    }
    return 0;
}
//2.libc库中提供的变量来获取
int main()
{
    //libc
    extern char** environ;
    for(int i=0;environ[i];i++)
    {
        printf("%s\n",environ[i]);
    }
    return 0;
}
#endif
int main()
{
    //libc库当中提供的函数来获取某一个具体的值
    //库函数
    //char *getenv(const char* name);
    //name:环境变量名称
    //返回值：具体环境变量对应的值
    char* PATH=getenv("PATH");
    printf("PATH=%s\n",PATH);
    return 0;
}
