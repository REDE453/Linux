#include<stdio.h>
#include<unistd.h>
#include<sys/shm.h>

#define shm_key 0x88888888
int main()
{
    int shmid=shmget(shm_key,1024,IPC_CREAT | 0064);
    if(shmid<0)
    {
        perror("shmget");
        return 0;
    }
    void* addr=shmat(shmid,NULL,0);
    if(!addr)
    {
        perror("shmat");
        return 0;
    }
    //while(1)
    {
        printf("readshm read:\"%s\"\n",(char*)addr);
        sleep(2);
    }
    //shmdt(addr);
    shmctl(shmid,IPC_RMID,NULL);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
