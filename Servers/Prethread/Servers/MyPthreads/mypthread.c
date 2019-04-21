#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <linux/sched.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "futex.c"
#include "fifo.c"
#include <sys/syscall.h>
#include "mypthread.h"
#include "threadList.c"
#include "cList.c"
#include "SRR.c"
#include "lottery.c"
#define STACK_SIZE 1024*1024
#define CLONE_SIGNAL            (CLONE_SIGHAND | CLONE_THREAD | CLONE_PTRACE)
 
struct sigaction stop;
struct futex gfutex;
int isSched = 0;

void hello(int signum){
  printf("Hello World!\n");
}

thread *pthread_self_pointer(){
    return search(pthread_self());
}

int adder(thread* node,int scheduler){
    if(isSched==1){
        return 0;
    }
    else if(scheduler==LOTTERY){
        lottery_schd->state = EXEC;
        addProcess(node,100);
        //printf("LOTTERY\n");
        //cdisplay(&lotteryList);
    }

    else if(scheduler==REAL){
        real_schd->state= EXEC;
    }
    else{
        srr_schd->state=EXEC;
        cadd(&newList,node);
    }
    
}

void setSchedPointer(thread* node, int scheduler){
    if(scheduler==LOTTERY){
        lottery_schd = node;
    }
    else if(scheduler==REAL){
        real_schd = node;
    }
    else{
        srr_schd = node;
    }
}


void *threadWrapper(void*  myThread){
    thread *new_tcb;
	new_tcb = (thread *) myThread;
    new_tcb->parentId = getpid();
   // printf("Pid = %d New Thread pid= %ld Stack=%p\n",getpid(),new_tcb->id,new_tcb->stack); 
	/* Suspend till explicitlysig woken up */
   // signal(WAIT,pthread_stop);
    futex_down(&new_tcb->sched);
	/* We have been woken up. Now, call the user-defined function */
	new_tcb->start_func(new_tcb->args);
	/* Ideally we shouldn't reach here */
	return 0;
}
/** ------------------------------------------------------
    * DOCPUBLIC
    *      The pthread_create() function is used to create a new thread,
    *      with attributes specified by attr, within a process
    *
    * PARAMETERS
    *      
    *
    *
    * DESCRIPTION
    *      
    *
    * RESULTS
    *              N/A
    *

* ------------------------------------------------------*/
int  pthread_create(pthread_t* pid,const pthread_attr_t *attr,void* threadFunction,void* arg,int scheduler){
    //child stack pointer
    char *child_stack;
    //stack size
    unsigned long stackSize;  

    //Pointer to new thread
    thread *new_node;

    //Return value
    int res;

    //Set flags
    int clone_flags = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGNAL
               | CLONE_PARENT_SETTID
               | CLONE_CHILD_CLEARTID | CLONE_SYSVSEM );

    // Beginning of time
    if(thread_head == NULL){
        res = initMainThread();
        if (res != 0){
            return res;
        }
        /* Initialise the global futex */
        futex_init(&gfutex, 1);

        /*Init scheduler*/
        isSched = 1;
        //pthread_create(&idFifo, NULL, fifo, NULL,0);
        //pthread_create(&idSRR,NULL,roundRobin,NULL,0);
        pthread_create(&idLottery,NULL,beginLottery,NULL,scheduler);
    }

    //Get memory address and space for the new node
    new_node = (thread *) malloc(sizeof(thread));
    if (new_node == NULL) {
        printf("Cannot allocate memory for node\n");
        return -1;
    }
    void *child = malloc(STACK_SIZE);
    child = child + STACK_SIZE;
    /* Save the thread_fun pointer and the pointer to arguments in the TCB. */
    new_node->start_func = threadFunction;
    new_node->args = arg;
    /* Set the state as READY - READY in Q, waiting to be scheduled. */
    new_node->state = READY;
    new_node->returnValue = NULL;
    new_node->blockedForJoin = NULL;
    new_node->stack = child;
    futex_init(&new_node->sched,0);
    tadd(new_node);
    if(isSched==1){
        setSchedPointer(new_node,scheduler);
    }
     /* Call clone with pointer to wrapper function. TCB will be passed as arg to wrapper function. */
    if ((*pid = clone(threadWrapper,child, clone_flags,new_node)) == -1) {
            printf("clone failed! \n");
            printf("ERROR: %d \n", strerror(errno));
            return (-errno);
    }
    //syscall(SYS_tgkill, getpid(),*pid, SIGTSTP); 
    /* Save the id returned by clone system call in the tcb. */
    new_node->id = *pid;
    adder(new_node,scheduler);
    isSched = 0;
    printf("pid %ld tid %d pid %d\n",*pid,getppid(),getpid());
    tdisplay();
    return 0;
} 

/*
* ------------------------------------------------------
    * DOCPUBLIC
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *
    * PARAMETERS
    *      value_ptr
    *              a generic data value (i.e. not the address of a value)
    *
    *
    * DESCRIPTION
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *      NOTE: thread should be joinable.
    *
    * RESULTS
    *              N/A
    *
* ------------------------------------------------------
*/
int initMainThread(){
    thread* main_node;
    main_node = (thread *) malloc(sizeof(thread));
    if (main_node == NULL) {
        printf("Cannot allocate memory for node\n");
        return -ENOMEM;
    }

    main_node->start_func = NULL;
    main_node->args = NULL;
    main_node->state = EXEC;
    main_node->returnValue = NULL;
    main_node->blockedForJoin = NULL;

    /* Get the main's tid and put it in its corresponding tcb. */
    main_node->id = pthread_self();
    /* Initialize futex to zero */
    futex_init(&main_node->sched, 1);
    /* Put it in the Queue of thread blocks */
    tadd(main_node);
    printf("Created Father %ld\n",main_node->id);
    return 0;
}
/*
* ------------------------------------------------------
    * DOCPUBLIC
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *
    * PARAMETERS
    *      value_ptr
    *              a generic data value (i.e. not the address of a value)
    *
    *
    * DESCRIPTION
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *      NOTE: thread should be joinable.
    *
    * RESULTS
    *              N/A
    *
* ------------------------------------------------------
*/
void pthread_exit(void* value_ptr){
    //get my thread
    pthread_t mythreadid = pthread_self();
    thread *currentThread = search(mythreadid);
    //change state to dead
    currentThread->state= DEFUNCT;
    //return value
    currentThread->returnValue = value_ptr;
    //check for blocked thread and unblocked
    if (currentThread->blockedForJoin != NULL){
        currentThread->blockedForJoin->state = EXEC;
        //kill(currentThread->blockedForJoin->id,SIGCONT);
    }
    //free memory 
    //free(currentThread->stack);
    printf("Dead thread %ld state %d\n", mythreadid,currentThread->state);
    //tdelete(currentThread);
    //cdelete(&newList,currentThread);
    tdisplay();
    //pthread_dispatch(currentThread);
    pthread_dispatch(currentThread);
    //exit process
    syscall(SYS_exit, 0);
     
    //tdelete(currentThread);
}


/*
* ------------------------------------------------------
    * DOCPUBLIC
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *
    * PARAMETERS
    *      value_ptr
    *              a generic data value (i.e. not the address of a value)
    *
    *
    * DESCRIPTION
    *      This function terminates the calling thread, returning
    *      the value 'value_ptr' to any joining thread.
    *      NOTE: thread should be joinable.
    *
    * RESULTS
    *              N/A
    *
* ------------------------------------------------------
*/
pthread_t pthread_self(){
    return (pid_t) syscall(SYS_gettid);
}

int pthread_dispatch(thread* myThread){
    thread* node = myThread->next;
    while(node->state!=EXEC){
        node = node->next;
        if(node->state==SLEEP && node==myThread){
            return -1;
        }
    }
    if(node==myThread && node->state==SLEEP){
        return -1;
    }
    else{
        futex_up(&node->sched);
        printf("Waking %ld\n",node->id);
        return 0;
    }
}




/*
Description:
Params:
Return:
*/
int pthread_yield(){
    int res;
    thread* self;
    self = pthread_self_pointer();
    //printf("Current--- %ld\n",self->id);
    futex_down(&gfutex);
    res = pthread_dispatch(self);
    if(res==-1){
        futex_up(&gfutex);
        return -1;
    }
    if(self->sched.count>0){
        futex_down(&self->sched);
    }
    
    futex_up(&gfutex);
    futex_down(&self->sched);
    return 0;
}

int pthread_yieldme(thread* thisThread){
    futex_down(&gfutex);
    
    if(thisThread->state!=EXEC){
        printf("No %ld THread %ld\n",thisThread->id,pthread_self());
        futex_up(&gfutex);
        return 0;
    }
    //printf("Execute %ld\n",thisThread->id);
    futex_up(&thisThread->sched);
    futex_up(&gfutex);
    //futex_down(&self->sched);
	return 0;   
}

void pthread_reset(){
    thread* head = search(getpid());
    pthread_dispatch(head);
}

void pthread_stop(int sig){
    printf("Enter THreadfin h\n");
    thread* thisThread = pthread_self_pointer();
    if(thisThread->stopped!=1){
        return;
    }
    printf("Ended %ld\n",thisThread->id);
    //futex_down(&gfutex);
    futex_down(&thisThread->sched);
    //futex_up(&gfutex);
    //
}

/*
Description:
Params:
Return:
*/
int pthread_join(pthread_t pthread, void **status){

    thread *target , *currentThread;
    currentThread = search(pthread_self());
   /// printf("target: Got tid: %ld\n", pthread);
    target = search(pthread);

    /* If the thread is already dead, no need to wait. Just collect the return
     * value and exit
     */
    if (target->state == DEFUNCT) {
        *status = target->returnValue;
        return 0;
    }

    //printf("Join: Checking for blocked for join\n");
    /* If the thread is not dead and someone else is already waiting on it
     * return an error
     */
    if (target->blockedForJoin != NULL){
        return -1;
    }

    /* Add ourselves as waiting for join on this thread. Set our state as
     * BLOCKED so that we won't be scheduled again.
     */
    target->blockedForJoin = currentThread;
    //printf("Join: Setting state of %ld to %d\n",
      //       (unsigned long)currentThread->id, BLOCKED);
    currentThread->state = BLOCKED;
    
    //waitpid(pthread,0,0);
    /* Schedule another thread */
    pthread_yield();
    /* Target thread died, collect return value and return */
    //*status = target->returnValue;
    return 0;
}
/*
Description:
Params:
Return:
*/
int pthread_detach(pthread_t thread){}
/*
Description:
Params:
Return:
*/
int pthread_init(pthread_mutex_t * thread, const pthread_mutexattr_t *attr){}
/*
Description:
Params:
Return:
*/
int pthread_destroy(pthread_mutex_t * thread){}
/*
Description:
Params:
Return:
*/
int pthread_lock(pthread_mutex_t * thread){}
/*
Description:
Params:
Return:
*/
int pthread_unlock(pthread_mutex_t * thread){}
/*
Description:
Params:
Return:
*/
int pthread_trylock(pthread_mutex_t * thread){}


