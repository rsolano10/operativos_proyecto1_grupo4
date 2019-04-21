#include "thread.h"
/* The global extern pointer defined in mythread.h which points to the head node in
   Queue of the Thread Control Blocks.
*/
thread *thread_head;


void init(thread * node)
{

	node->prev = node;
	node->next = node;

	thread_head = node;

}

/* This function adds a node to the Queue, at the end of the Queue. 
   This is equivalent to Enque operation.
 */
void tadd(thread * node)
{

	if (thread_head == NULL) {
		//Q is not initiazed yet. Create it.
		init(node);
		return;
	}
	//Insert the node at the end of Q
	node->next = thread_head;
	node->prev = thread_head->prev;
	thread_head->prev->next = node;
	thread_head->prev = node;
	return;

}

/* This function deleted a specified(passed as a parameter) node from the Queue.
 */
void tdelete(thread * node)
{

	thread *p;
	if (node == thread_head && node->next == thread_head) {
		//There is only a single node and it is being deleted
		printf("The Q is now Empty!\n");
		thread_head = NULL;
	}

	if (node == thread_head)
		thread_head = node->next;

	p = node->prev;

	p->next = node->next;
	node->next->prev = p;

	return;

}

/* This function iterates over the ntire Queue and prints out the state(see mythread.h to refer to various states)
   of all the tcb members.
 */
void tdisplay()
{

	if (thread_head != NULL) {

		//display the Q - for debug purposes
		printf("\n The Q contents are -> \n");
		thread *p;
		p = thread_head;
		do {		//traverse to the last node in Q
			printf("Thread %ld ,state %d\n",p->id, p->state);
			p = p->next;
		} while (p != thread_head);

	}

}

/* This function iterates over the Queue and prints out the state of the specified thread.
 */
thread *search(unsigned long new_tid)
{

	thread *p;
	if (thread_head != NULL) {

		p = thread_head;
		do {		//traverse to the last node in Q
			if (p->id == new_tid)
				return p;
			p = p->next;
		} while (p != thread_head);

	}
	return NULL;

}