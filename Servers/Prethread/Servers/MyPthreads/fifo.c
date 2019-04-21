  
/* Single Author info:
 * 	jhshah 	 Jitesh H Shah
 * Group info:
 * 	jhshah	 Jitesh H Shah
 * 	sskanitk Salil S Kanitkar
 * 	ajalgao	 Aditya A Jalgaonkar
 */

#include <unistd.h>
#include "mypthread.h"
#include "futex.h"

/* Idle thread implementation. 
 * The thread checks whether it is the only one alive, if yes, exit()
 * else keep scheduling someone.
 */
void *fifo(void *phony)
{

	thread *traverse_tcb;
	pthread_t idle_tcb_tid;
   
	while (1) {
		printf("Start FIFO scheduler\n");
		traverse_tcb =(thread*)pthread_self_pointer();
		idle_tcb_tid = traverse_tcb->id;
		traverse_tcb = traverse_tcb->next;
		/* See whether there is a NON-DEFUNCT process in the list.
		 * If there is, idle doesn't need to kill the process just yet */
		while (traverse_tcb->id != idle_tcb_tid) {
			printf("process %ld my id %ld\n",traverse_tcb->id,pthread_self());		
			if (traverse_tcb->state != DEFUNCT) {
				break;
			}
			
			traverse_tcb = traverse_tcb->next;
		}

		/* Idle is the only one alive, kill the process */
		if (traverse_tcb->id == idle_tcb_tid){
			printf("Bye\n");
			exit(0);
		}
		
		/* Some thread still awaits execution, yield ourselves */
		printf("Next Process\n");
		pthread_yield();
	}
}