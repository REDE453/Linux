#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc,char* argv[],char* env[])
{
    printf("我是myexec函数，我要开始执行了\n");
    for(int i=0;env[i];i++)
    {
        printf("%s\n",env[i]);
        printf("\n");
    }
    printf("我是myexec可执行程序，我已经打印完毕环境变量了\n");
    for(int i=0;i<argc;i++)
    {
        printf("parm[%d]:%s\n",i,argv[i]);
    }
    printf("我是myexec可执行程序，我已经打印完毕命令行参数了\n");
    while(1)
    {
        printf("hahahah\n");
        sleep(1);
    }
    return 0;
}
