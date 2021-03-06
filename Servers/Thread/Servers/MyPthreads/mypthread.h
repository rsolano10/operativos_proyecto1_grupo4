#include <sys/types.h>
#include "thread.h"	

#ifndef MYPTHREAD
#define MYPTHREAD

// 64kB stack
int initMainThread();
int pthread_create(pthread_t* pid,const pthread_attr_t *attr,void *threadFunction,void* arg);
void pthread_exit(void* value_ptr);
int pthread_yield();
pthread_t pthread_self();
int pthread_join(pthread_t thread, void **status);
int pthread_detach(pthread_t thread);
int pthread_init(pthread_mutex_t *thread, const pthread_mutexattr_t *attr);
int pthread_destroy(pthread_mutex_t *thread);
int pthread_lock(pthread_mutex_t *thread);
int pthread_unlock(pthread_mutex_t *thread);
int pthread_trylock(pthread_mutex_t *thread);


#endif