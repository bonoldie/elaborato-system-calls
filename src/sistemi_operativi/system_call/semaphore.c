/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "err_exit.h"
#include "defines.h"
#include "semaphore.h"

//ID Ipc
int CLIENTSemId = -1;
int FIFO1SemId = -1;
int FIFO2SemId = -1;
int ShmSemId = -1;
int MsgQueueSemId = -1;

void semOp(int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        ErrExit("semop failed");
}

int getSemValues(int semid, short *values)
{
    union semun arg;
    arg.array = values;

    return semctl(semid, 0, GETALL, arg);
}

int setSemValues(int semid, short * initValues)
{
    union semun arg;
    arg.array = initValues;

    return semctl(semid, 0, SETALL, arg);
}

void setupSemaphores()
{
    FIFO1SemId = semget(FIFO1_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    FIFO2SemId = semget(FIFO2_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    ShmSemId = semget(SHM_PRIVATE, 50, IPC_CREAT | S_IRUSR | S_IWUSR);
    MsgQueueSemId = semget(MSGQUEUE_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    

    if (ShmSemId == -1 || FIFO1SemId == -1 || FIFO2SemId == -1 || MsgQueueSemId == -1){
        ErrExit("<setupSemaphores> semget failed");
    }
}

void initSemaphores()
{
    short _[1] = {1};
    short __[50] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};
    short ___[1] = {50};
  
    if (setSemValues(FIFO1SemId, &_) == -1 || setSemValues(FIFO2SemId, &_) == -1 || setSemValues(ShmSemId, &__) == -1 || setSemValues(MsgQueueSemId, &___) == -1)
    {
        ErrExit("<initSemaphores> semctl failed");
    }
}


void initClientSemaphore(int filePathsCounter){
    
  CLIENTSemId = semget(CLIENT_PRIVATE, filePathsCounter, IPC_CREAT | S_IRUSR | S_IWUSR);

  if (CLIENTSemId == -1){
        ErrExit("<setupSemaphores> semget failed");
    }
  
  int i;
  short ____[i];
  
  for(i = 0; i < filePathsCounter; ++i){
       ____[i] = 0;
    }
  ____[i - 1] = 1;

    if (setSemValues(CLIENTSemId,&____) == -1)
    {
        ErrExit("<initSemaphores> semctl failed");
  
}
  
  }

void printSemValues(int semid, int filePathsCounter) // = 1 in FIFO
{
    int semLength = 0;

    if (semid == FIFO1SemId || semid == FIFO2SemId ||  semid == MsgQueueSemId )
    {
        semLength = 1;
    }
    else if(semid == CLIENTSemId){
      semLength = filePathsCounter;
    }
    else
    {
        semLength = 50;
    }

    unsigned short semVal[semLength];
    union semun arg;
    arg.array = semVal;

    printf("%hu \n", semid);

    // get the current state of the set
    if (semctl(semid, 0, GETALL, arg) == -1)
        ErrExit("semctl GETALL failed");

    // print the semaphore's value
    printf("Semaphore set state:\n");
    for (int i = 0; i < semLength; i++)
        printf("    id: %d --> %d\n", i, semVal[i]);
}
