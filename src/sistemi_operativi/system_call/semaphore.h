/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

// definition of the union semun
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

/* semOp is a support function to manipulate a semaphore's value
 * of a semaphore set. semid is a semaphore set identifier, sem_num is the
 * index of a semaphore in the set, sem_op is the operation performed on sem_num
 */
void semOp (int semid, unsigned short sem_num, short sem_op);

int getSemValues(int semid, short * values);

int setSemValues(int semid, short * initValues);

// Semaphores
extern int FIFO1SemId;
extern int FIFO2SemId;
extern int ShmSemId;
extern int MsgQueueSemId;
extern int CLIENTSemId;

void initSemaphores();
void setupSemaphores();


void printSemValues(int semid);

#endif
