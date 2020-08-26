#include<stdio.h>
#include<unistd.h>

int main()
{
    int aa=10;
    int pid = fork();
    if(pid<0)
    {
        perror("fork");
        return 0;
    }
    else if(pid==0)
    {
        //child
        aa+=10;
        printf("i am child aa=[%d][%p]\n",aa,&aa);
    }
    else
    {
        //father
        aa-=10;
        printf("i am father aa=[%d][%p]\n",aa,&aa);
    }
    return 0;
}
