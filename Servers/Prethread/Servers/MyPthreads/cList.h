#include <sys/types.h>
#include "thread.h"	

#ifndef CLIST
#define CLIST

// 64kB stack
void cinit(thread **head_ref, thread *node);
void cadd(thread **head_ref, thread *node);
void cdelete(thread **head_ref, thread *node);
void cdisplay(thread **head_ref);
thread *csearch(thread **head_ref, unsigned long new_tid);
void cdeletebyId(thread **head_ref, unsigned long tid);
thread *cgetItem(thread **head_ref, int position);
int cLength(thread **head_ref);
#endif