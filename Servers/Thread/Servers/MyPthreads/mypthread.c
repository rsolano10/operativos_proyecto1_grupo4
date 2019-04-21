#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <linux/sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 
#include <sys/syscall.h>
#include "mypthread.h"
#include "threadList.c"

#define CLONE_SIGNAL            (CLONE_SIGHAND | CLONE_THREAD)
 

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
int  pthread_create(pthread_t* pid,const pthread_attr_t *attr,void* threadFunction,void* arg){
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
               | CLONE_CHILD_CLEARTID | CLONE_SYSVSEM);

    // Beginning of time
    if(thread_head == NULL){
        res = initMainThread();
        if (res != 0){
            return res;
        }
        /* Initialise the global futex */
        //futex_init(&gfutex, 1);

        /*Init scheduler*/
        //mythread_create(&idle_u_tcb, NULL, mythread_idle, NULL);
    }

    //Get memory address and space for the new node
    new_node = (thread *) malloc(sizeof(thread));
    if (new_node == NULL) {
        printf("Cannot allocate memory for node\n");
        return -1;
    }

    /* If Stack-size argument is not provided, use the SIGSTKSZ as the default stack size 
     * Otherwise, extract the stacksize argument.
     */
    if (attr == NULL){
        stackSize = SIGSTKSZ;
    }
    else{
        //stackSize = attr->stacksize;
    }

    /* posix_memalign aligns the allocated memory at a 64-bit boundry. */
    if (posix_memalign((void **)&child_stack, 8, stackSize)) {
        printf("posix_memalign failed! \n");
        return -1;
    }

    /* Save the thread_fun pointer and the pointer to arguments in the TCB. */
    new_node->start_func = threadFunction;
    new_node->args = arg;
    /* Set the state as READY - READY in Q, waiting to be scheduled. */
    new_node->state = NEW;

    new_node->returnValue = NULL;
    new_node->blockedForJoin = NULL;
    new_node->stack = child_stack;
    /* We leave space for one invocation at the base of the stack */
    child_stack = child_stack + stackSize - sizeof(sigset_t);
    
    tadd(new_node);

     /* Call clone with pointer to wrapper function. TCB will be passed as arg to wrapper function. */
    if ((*pid = clone(threadFunction, (char *)child_stack, clone_flags,arg)) == -1) 
    {
        printf("clone failed! \n");
        printf("ERROR: %d \n", strerror(errno));
        return (-errno);
    }
    //printf("pid %ld\n",*pid);
    //kill(*pid,SIGSTOP);  


    /* Save the id returned by clone system call in the tcb. */
    new_node->id = *pid;

    printf("Pid = %d New Thread pid= %ld Stack=%p\n",getpid(),new_node->id,new_node->stack); 
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
    main_node->state = READY;
    main_node->returnValue = NULL;
    main_node->blockedForJoin = NULL;

    /* Get the main's tid and put it in its corresponding tcb. */
    main_node->id = getpid();

    /* Initialize futex to zero */
    //futex_init(&main_node->sched_futex, 1);

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
        currentThread->blockedForJoin->state = READY;
        //kill(currentThread->blockedForJoin->id,SIGCONT);
    }
    //free memory 
    free(currentThread->stack);
    //exit process
    syscall(SYS_exit, 0);

    //printf("Dead thread %ld\n", mythreadid);  
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

/*
Description:
Params:
Return:
*/
int pthread_yield(pthread_t thread){}
/*
Description:
Params:
Return:
*/
int pthread_join(pthread_t pthread, void **status){

    thread *target , *currentThread;
    currentThread = search(pthread_self());
    printf("target: Got tid: %ld\n", pthread);
    target = search(pthread);

    /* If the thread is already dead, no need to wait. Just collect the return
     * value and exit
     */
    if (target->state == DEFUNCT) {
        *status = target->returnValue;
        return 0;
    }

    printf("Join: Checking for blocked for join\n");
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
    printf("Join: Setting state of %ld to %d\n",
             (unsigned long)currentThread->id, BLOCKED);
    currentThread->state = BLOCKED;
    //kill(currentThread->id,SIGSTOP);
    waitpid(pthread,0,0);
    /* Schedule another thread */
    //mythread_yield();

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


