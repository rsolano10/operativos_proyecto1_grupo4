#include "sys/types.h"

#ifndef THREAD_H
#define THREAD_H

#define FALSE 0
#define TRUE 1
#define RUNNING 0
#define READY 	1 /* Ready to be scheduled */
#define BLOCKED 2 /* Waiting on Join */
#define DEFUNCT 3 /* Dead */
#define NEW 4 /* New */



typedef struct thread
{
  pthread_t id; 				/* The thread-id of the thread */
  char* stack;
  int state; 				/* the state in which the corresponding thread will be. */
  void * (*start_func) (void *); 	/* The func pointer to the thread function to be executed. */
  void *args; 				/* The arguments to be passed to the thread function. */
  void *returnValue; 			/* The return value that thread returns. */
  struct thread *blockedForJoin; 	/* Thread blocking on this thread */
  struct thread *prev, *next;
} thread;

#endif