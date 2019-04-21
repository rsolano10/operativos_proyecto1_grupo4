//Imports
#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "sys/types.h"
#include <time.h>
#include "cList.c"

//SRR defines for implementation
#define TIMEACCEPTED 1000

//list heads
thread *RTList = NULL;
thread *highPriority = NULL;

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds)
        ;
}

void softRealTime()
{
    while(1)
    {
        int len = cLength(&RTList);

        /*Verify high priority thread based on size of offset*/
        clock_t start_time = clock();
        for (int i = 0; i < (len - 1); i++)
        {
            clock_t end_time = clock();
            if ((start_time - end_time) > (TIMEACCEPTED / 2))
            {
                break;
            }
            thread *temp0 = cgetItem(&RTList, i);
            thread *temp1 = cgetItem(&RTList, i + 1);
            if (temp0->offset >= temp1->offset)
            {
                highPriority = temp1;
            }
            else
            {
                highPriority = temp0;
            }
        }
        
        //validate the case the list have only one process
        if (len == 1)
        { 
            highPriority = cgetItem(&RTList, 0);
        }

        /*EXEC Proccess*/
        if (len > 0)
        {
            if (highPriority->state != DEFUNCT)
            {
                /*Verify process is not blocked*/
                if (highPriority->state != BLOCKED)
                {
                    highPriority->state = EXEC;
                    //thread.yield();
                    delay(TIMEACCEPTED);
                    highPriority->state = READY;
                }
            }
            else
            {
                cdelete(&RTList, highPriority);
            }
        }
    }
}

int main()
{
    struct thread *t1 = malloc(sizeof(struct thread));
    t1->id = getpid();
    t1->offset = 1;
    t1->state = NEW;

    struct thread *t2 = malloc(sizeof(struct thread));
    t2->id = getpid() + 1;
    t2->offset = 3;
    t2->state = NEW;

    cadd(&RTList, t1);
    cadd(&RTList, t2);

    cdisplay(&RTList);

    softRealTime();

    cdisplay(&RTList);

    return 0;
}