//Imports
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cList.h"

/**
 * head_ref = reference of head of list
 * node = new node to instance
**/
void cinit(thread **head_ref, thread *node)
{

    node->cprev = node;
    node->cnext = node;

    *head_ref = node;
}

/* This function adds a node to the Queue, at the end of the Queue. 
   This is equivalent to Enque operation.
 */
void cadd(thread **head_ref, thread *node)
{

    if (*head_ref == NULL)
    {
        //Q is not initiazed yet. Create it.
        cinit(head_ref, node);
        return;
    }
    //Insert the node at the end of Q
    node->cnext = *head_ref;
    node->cprev = (*head_ref)->cprev;
    (*head_ref)->cprev->cnext = node;
    (*head_ref)->cprev = node;
    return;
}

/* This function deleted a specified(passed as a parameter) node from the Queue.
 */
void cdelete(thread **head_ref, thread *node)
{

    thread *p;
    if (node == (*head_ref) && node->cnext == (*head_ref))
    {
        //There is only a single node and it is being deleted
        //printf("The Q is now Empty!\n");
        (*head_ref) = NULL;
    }

    if (node == (*head_ref))
        (*head_ref) = node->cnext;

    p = node->cprev;

    p->cnext = node->cnext;
    node->cnext->cprev = p;

    return;
}

/* This function iterates over the ntire Queue and prints out the state(see mythread.h to refer to various states)
   of all the tcb members.
 */
void cdisplay(thread **head_ref)
{

    if ((*head_ref) != NULL)
    {

        //display the Q - for debug purposes
        printf("\n The Q contents are -> \n");
        thread *p;
        p = (*head_ref);
        do
        { //traverse to the last node in Q
            printf("Thread %ld ,state %d, priority %i\n", p->id, p->state, p->priority);
            p = p->cnext;
        } while (p != (*head_ref));
    }
    else{
        printf("\n The Q is empty \n");
    }
}

/* This function iterates over the Queue and prints out the state of the specified thread.
 */
thread *csearch(thread **head_ref, unsigned long new_tid)
{

    thread *p;
    if ((*head_ref) != NULL)
    {

        p = (*head_ref);
        do
        { //traverse to the last node in Q
            if (p->id == new_tid)
                return p;
            p = p->cnext;
        } while (p != (*head_ref));
    }
    return NULL;
}

void cdeletebyId(thread **head_ref, unsigned long tid)
{

    thread *node, *p;
    if ((*head_ref) != NULL)
    {

        node = (*head_ref);
        do
        { //traverse to the last node in Q
            if (node->id == tid)
            {
                if (node == (*head_ref) && node->cnext == (*head_ref))
                {
                    //There is only a single node and it is being deleted
                    printf("The Q is now Empty!\n");
                    (*head_ref) = NULL;
                }

                if (node == (*head_ref))
                    (*head_ref) = node->cnext;

                p = node->cprev;

                p->cnext = node->cnext;
                node->cnext->cprev = p;
                return;
            }
        } while (node != (*head_ref));
    }

    return;
}

thread *cgetItem(thread **head_ref, int position)
{
    int cont = 0;
    if (position == 0)
    {
        return (*head_ref);
    }
    else
    {
        thread *p;
        p = (*head_ref);
        do
        { //traverse to the last node in Q
            if (cont == position)
            {
                return p;
            }
            p = p->cnext;
            cont++;
        } while (p != (*head_ref));
    }
    return NULL;
}

int cLength(thread **head_ref)
{
    int cont = 0;
    thread *p;
    if ((*head_ref) != NULL)
    {

        p = (*head_ref);
        do
        {
            p = p->cnext;
            cont++;
        } while (p != (*head_ref));
    }
    return cont;
}
