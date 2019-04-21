#include "sys/types.h"
#include <time.h>

#ifndef THREAD_H
#define THREAD_H

#define FALSE 0
#define TRUE 1

#define WAIT 200
#define SRR 0
#define LOTTERY 1
#define REAL 2
#define RUNNING 0
#define READY 	1 /* Ready to be scheduled */
#define BLOCKED 2 /* Waiting on Join */
#define DEFUNCT 3 /* Dead */
#define NEW 4 /* New */
#define EXEC 5 /* To execute by yield */
#define SLEEP 6 /* To execute by yield */

/* You should never touch this structure directly! */

typedef struct thread
{
  int stopped;
  pthread_t id; 				/* The thread-id of the thread */
  pthread_t parentId;
  char* stack;
  int tickets[100]; //For array of tickets by process fir lottery scheduler
  int state; 				/* the state in which the corresponding thread will be. */
  void * (*start_func) (void *); 	/* The func pointer to the thread function to be executed. */
  void *args; 				/* The arguments to be passed to the thread function. */
  void *returnValue; 			/* The return value that thread returns. */
  struct futex sched;
  struct thread *blockedForJoin; 	/* Thread blocking on this thread */
  struct thread *prev, *next, *cprev, *cnext;
  int priority;
  clock_t start_time;
  long offset;

} thread;



#endif