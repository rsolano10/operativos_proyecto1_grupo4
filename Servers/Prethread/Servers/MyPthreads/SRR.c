
//Imports
#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "sys/types.h"
#include <time.h>
//#include "cList.h"
#include "mypthread.h"

//SRR defines for implementation
#define INCREMENT_NEW 2
#define INCREMENT_ACCEPTED 1
#define QUANTUM 1

//list heads
thread *newList = NULL;
thread *acceptedList = NULL;

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}

void *roundRobin(void* arg)
{   
    while (1)
    {
        /*first step verification in case accepted list is empty*/
        if (cLength(&acceptedList) == 0)
        {
            /*first step verification in case there exists processes inside new list */
            if (cLength(&newList) > 0)
            {
                /*delete from one list and insert into the other*/
                thread *temp = cgetItem(&newList, 0);
                if(temp->id!=0)
                {
                    if(temp->state!=DEFUNCT){
                        temp->state = READY; // change state of the thread
                        cdelete(&newList, temp);
                        cadd(&acceptedList, temp);
                    }
                }
            }
        }

        /*verify it exists process to execute*/
        if (cLength(&acceptedList) > 0)
        {
            thread *toExecItem = cgetItem(&acceptedList, 0);

            /*verify process is not defunct*/
            if (toExecItem->state == DEFUNCT)
            {
                cdelete(&acceptedList, toExecItem);
            }

            /*Verify the thread is not blocked*/
            if (toExecItem->state != BLOCKED)
            {
                /*Ejecuta proceso por determinado tiempo*/
                if(toExecItem->state!=DEFUNCT){
                    toExecItem->state = EXEC;
                    pthread_yieldme(toExecItem);}
                    //delay(QUANTUM);
                    //futex_down(&toExecItem->sched);
                    toExecItem->state = READY;
                    //pthread_yield();
                    //printf("change\n");
                    /*send to the back of the list*/
                    cdelete(&acceptedList, toExecItem);
                    cadd(&acceptedList, toExecItem);
                }
            }

            /*increase priority of list accepted*/
            for (int i = 0; i < cLength(&acceptedList); i++)
            {
                thread *temp = cgetItem(&acceptedList, i);
                if (temp->state != BLOCKED)
                {
                    temp->priority += INCREMENT_ACCEPTED;
                }
            }
        }

        /*increase priority of List New*/
        if (cLength(&newList) > 0)
        {
            for (int i = 0; i < cLength(&newList); i++)
            {
                thread *temp = cgetItem(&newList, i);
                temp->priority += INCREMENT_NEW;
            }
        }

        /*Eval priorities*/
        thread *itemaccepted = cgetItem(&acceptedList, 0);
        thread *itemready = cgetItem(&newList, 0);
        if (itemaccepted > 0 && itemready > 0)
        {
            if (itemaccepted->priority <= itemready->priority)
            {

                /*verify process is not defunct*/
                if (itemready->state == DEFUNCT)
                {
                    cdelete(&newList, itemready);
                }

                /*Verify the thread is not blocked*/
                if (itemready->state != BLOCKED || itemready->id!=0)
                {
                    /*Ejecuta proceso por determinado tiempo*/
                    if(itemready->state!=DEFUNCT){
                        itemready->state = EXEC;
                        pthread_yieldme(itemready);
                        //delay(QUANTUM);
                        //futex_down(&itemready->sched);
                        itemready->state = READY;
                        //pthread_yield();
                        //printf("change 2\n");
                        /*delete from one list and insert into the other*/
                        cdelete(&newList, itemready);
                        cadd(&acceptedList, itemready);
                    }

                    /*increase priority of list accepted*/
                    for (int i = 0; i < cLength(&acceptedList); i++)
                    {
                        thread *temp = cgetItem(&acceptedList, i);
                        if (temp->state != BLOCKED)
                        {
                            temp->priority += INCREMENT_ACCEPTED;
                        }
                    }

                    /*increase priority of List New*/
                    if (cLength(&newList) > 0)
                    {
                        for (int i = 0; i < cLength(&newList); i++)
                        {
                            thread *temp = cgetItem(&newList, i);
                            temp->priority += INCREMENT_NEW;
                        }
                    }
                }
            }
        }
    }
}
