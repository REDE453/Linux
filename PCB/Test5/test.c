#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc,char* argv[])
{
    extern  char** environ;
    printf("我是test可执行程序\n");
    for(int i=0;environ[i];i++)
    {
        printf("%s\n",environ[i]);
        printf("\n");
    }
    printf("我是TEST可执行程序，打印完环境变量了\n");
    for(int i=0;i<argc;i++)
    {
        printf("pram[%d]:%s\n",i,argv[i]);
    }
    printf("我是可执行程序打印完命令行参数了\n");
    sleep(5);
    //exec l系列函数
    //execl("./exec","exec","-l",NULL);
    //execlp("/home/linux/Linux/PCB/Test5/myexec","myexec","-l",NULL);
    //char* envp[]={"MYENV=xxxx","HEHE=yyyy",NULL};
    //execle("/home/linux/Liunx/PCB/Test5/myexec","myexec","-l",NULL,envp);
    //exec v系列函数
    char* argv1[]={"myexec","-l","-hello","-123456789",NULL};

    //execv("/home/linux/Linux/PCB/Test5/myexec",argv1);
    execv("myexec",argv1);
    perror("execl");
    printf("hehehe\n");
    exit(-1);
    return 0;
}
