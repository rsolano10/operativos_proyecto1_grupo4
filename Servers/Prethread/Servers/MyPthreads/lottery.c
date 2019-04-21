#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "mypthread.h"
//#include "cList.c"

long ticketCount = 0;
thread *lotteryList = NULL; //Array for all lottery process
int seed = 0; // seed for random number
int emptyFlag = 0; // flag for empty lottery array process
// int hh = 1; /*** FOR TESTING ***/


// Add tickets to each new process using the size of file like priority
void addProcess(thread *process, long fileSize)
{
    int numTickets = 0;
    if (fileSize <= 100)
    {
        numTickets = 100;
    }
    else if (fileSize > 100 && fileSize <= 1000)
    {
        numTickets = 75;
    }
    else if (fileSize > 1000 && fileSize <= 1000000)
    {
        numTickets = 50;
    }
    else if (fileSize > 1000000 && fileSize <= 1000000000)
    {
        numTickets = 25;
    }
    else if (fileSize > 1000000000)
    {
        numTickets = 10;
    }
    for (int i = 0; i < numTickets; i++)
    {
        process->tickets[i] = ticketCount;
        ticketCount++;
    }
    process->state = READY;
    emptyFlag = 0;
    cadd(&lotteryList, process);
}

// Main process for select a winner ticket and execute that process
void execProcess()
{
    if (!emptyFlag)
    {
        srand(seed * time(0));
        long winnerNumber = rand() % ticketCount;
        //printf("WINNER NUMBER: %ld\n", winnerNumber);
        int numProcess = cLength(&lotteryList);
        //printf("NUM PROCESS: %d\n", numProcess);
        int flag = 0;
        unsigned long winnerProcess = 0;
        thread *ob;
        ob = (thread *)malloc(sizeof(thread));
        for (int i = 0; i < numProcess; i++)
        {
            ob = cgetItem(&lotteryList, i);
            for (int j = 0; j < 100; j++)
            {
                if (ob->tickets[j] == 0 && j != 0)
                {
                    flag = 0;
                    break;
                }
                else if (ob->tickets[j] == winnerNumber)
                {
                    winnerProcess = ob->id;
                    flag = 1;
                    break;
                }
            }
            if (flag == 1)
            {
                break;
            }
        }
        if (flag == 0)
        {
            printf("Ticket not found!\n\n");
        }
        else
        {
            ob->state = EXEC;
            pthread_yield();
            thread *ob1 = csearch(&lotteryList, winnerProcess);
            //ob1->state = DEFUNCT;
            if (ob1->state == DEFUNCT)
            {
               // printf("BORRANDO\n");
                cdelete(&lotteryList, ob1);
                int numero = cLength(&lotteryList);
               // printf("DESPUES DE BORRAR: %d\n", numero);
                if (numProcess == 1)
                {
                    emptyFlag = 1;
                }
            }
            //printf("EXECUTED PROCESS: %ld\n\n", winnerProcess);
        }
    }
    else{
        printf("Sleep Lottery\n");
        pthread_self_pointer()->state = SLEEP;
        if(pthread_yield()==-1){
            exit(0);
        }
    }
}


// Loop for lottery scheduler
void *beginLottery(void* arg)
{
    while (1)
    {
        execProcess();
        if (!emptyFlag)
        {
            seed++;
        }
        if(seed>50000){
            seed=0;
        }
    }

    /***** FOR TESTING ****/
    // int cont = 0;
    // while (cont < 1000000)
    // {
    //     execProcess();
    //     if (!emptyFlag)
    //     {
    //         seed++;
    //     }
    //     if (cont > 200 && hh)
    //     {
    //         seed = 0;
    //         thread *c;
    //         c = (thread *)malloc(sizeof(thread));
    //         c->id = 4444;
    //         c->state = NEW;
    //         addProcess(c, 1111);
    //         thread *a;
    //         a = (thread *)malloc(sizeof(thread));
    //         a->id = 5555;
    //         a->state = NEW;
    //         addProcess(a, 64654);
    //         hh = 0;
    //     }
    //     cont++;
    // }
}


/***** FOR TESTING ****/
// int main()
// {   
//     thread *a;
//     a = (thread *)malloc(sizeof(thread));
//     a->id = 1111;
//     a->state = NEW;
//     thread *b;
//     b = (thread *)malloc(sizeof(thread));
//     b->id = 2222;
//     b->state = NEW;
//     thread *c;
//     c = (thread *)malloc(sizeof(thread));
//     c->id = 3333;
//     c->state = NEW;
//     addProcess(a, 64654);
//     addProcess(b, 65616845);
//     addProcess(c, 1);
//     beginLottery();
//     return 1;
// }
