#include "mypthread.c"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <signal.h>
//#include <pthread.h>

void *func(void* pData){
    pthread_t mythread = pthread_self();
    int n;
    n=10;
    FILE *pFile;
    pFile = fopen((char*)pData,"w");
    if (pFile!=NULL){
        for (int i = 0; i < n; ++i)
        {  
           fprintf(pFile,"Thread %ld iteration %d\n", pthread_self(),i);
        }
        if((char*)pData=="1.txt"){
            for (int i = 0; i < 1; ++i)
            {
                printf("I: %d\n",i);
            }
        }   
        if((char*)pData=="2.txt"){
            for (int i = 0; i < 1; ++i)
            {
               printf("perra 2\n");
            }
        }   
    }
    fclose(pFile);    
    pthread_exit(NULL);
}

int main()
{   
    //printf("Thread %d Parent %d\n", getpid(),getppid());
    pthread_t pid;
    pthread_t pid1;
    pthread_t pid2;
    
    pthread_create(&pid,NULL,func,"1.txt",1);
    pthread_create(&pid1,NULL,func,"2.txt",1);
    pthread_join(pid,NULL);
    //pthread_create(&pid2,NULL,func,"1");
    //printf("pid  %ld ppid %ld\n",getpid(),getppid());
    //pthread_join(pid,NULL);
    //pthread_join(pid,NULL);
    //while(1){}
    pthread_exit(NULL);
    return 0;
}
