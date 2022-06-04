// @file client.c
// @brief Contiene l'implementazione del client.
// kill -SIGKILL $(ps -aux | grep -m1 client_0 | cut -d' ' -f6) $(ps -aux | grep -m1 client_0 | cut -d' ' -f7)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>

#include <signal.h>

#include <unistd.h>
#include <errno.h>

#include <pwd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include <dirent.h>

#include "err_exit.h"
#include "fs.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

struct MemoryDisposition *shmDisposition;
struct SerializedMessage msgqueueMsg;

void startComunication();
void endComunication();

void welcomeMessage();

void blockAllSignals();
void blockSomeSignals();

short FIFO1SemValue[1];
short FIFO2SemValue[1];
short MsgQueueSemValue[1];
short ShmSemValue[50];

char CWDARG[250];

char *filePaths[MAX_FILES];
int filePathsCounter;

int MsgQueueId = -1;



void sigHandler(int sig)
{

    if (sig == SIGINT)
    {
      blockAllSignals();
      startComunication();
      blockSomeSignals();
    }
    else if (sig == SIGUSR1)
    {
        endComunication();
    }
}

int main(int argc, char *argv[])
{
    // imposto directory corrente
    if (argc != 2)
    {
        printf("Usage: \n");
        printf("%s <directory> \n", argv[0]);
        return 1;
    }

    strcat(CWDARG, argv[1]);

    // set the function sigHandler for signals
    if (signal(SIGINT, sigHandler) == SIG_ERR ||
        signal(SIGUSR1, sigHandler) == SIG_ERR)
    {
        ErrExit("change signal handler failed");
    }

    shmDisposition = init_shared_memory();

    setupSemaphores();

    while (1)
    {

        sleep(10);
    }

    return 0;
}

void startComunication()
{
   
    initSemaphores();

    // imposto CWD
    chdir(CWDARG);

    welcomeMessage();
  
    printf("%d", MsgQueueId);

    filePathsCounter = 0;
    loadFilePaths(".", filePaths, &filePathsCounter);

    initClientSemaphore(filePathsCounter);

    // FIFO 1 CLIENT
    printf("<Client> Opening FIFO1 ...\n");
    // Open the FIFO in write-only mode
    int FIFO1 = getFIFO(FIFO1PATH, O_RDWR);
    int FIFO2 = getFIFO(FIFO2PATH, O_RDWR);

    MsgQueueId = getMsgQueue(IPC_CREAT | S_IRUSR | S_IWUSR);

  size_t mSize = sizeof(struct SerializedMessage) - sizeof(long);
  
    printf("<Client> Waiting for serverOk\n");

    printf("<Client> sending %i\n", filePathsCounter);
    // Wrinte  two integers to the opened FIFO
    if (write(FIFO1, &filePathsCounter, sizeof(int)) <= 0)
        ErrExit("write failed");

    while (shmDisposition->serverOk == 0)
    {
        sleep(1);
    }

    shmDisposition->serverOk = 0;

    printf("<Client> Received serverOk\n");

    fflush(stdout);

    int filemandati[4] = {0,0,0,0};

    // Valore sem client N dei path
    //printSemValues(CLIENTSemId);

    // Creazione dei figli
    pid_t pid;

    for (int i = 0; i < filePathsCounter; ++i)
    {

        int code = (int)(((double)rand() / RAND_MAX) * 255);
        pid = fork();
        if (pid == -1)
        {
            printf("Child %d not created!", i);
        }
        else if (pid == 0)
        {
            struct ApplicationMsg messages[4];
            char serialized[4][MESSAGE_SIZE];
          
            buildMessages(filePaths[i], messages);

            for (int j = 0; j < 4; ++j)
            {
            serializeMessage(&(messages[j]), &(serialized[j]));
            //    printf("%s \n", &(serialized[j]));
            }
            //printSemValues(CLIENTSemId);
            //sem da 50 a 0
            semOp(CLIENTSemId, 0, -1);
            //sem da 0 a 50
            semOp(CLIENTSemId, 1, -1);
            
            // printf("PIDchild: %d, PIDparent: %d\n", getpid(), getppid());
          

          while (filemandati[0] == 0 ||  filemandati[1] == 0 || filemandati[2] == 0 || filemandati[3] == 0){

            getSemValues(FIFO1SemId, FIFO1SemValue);
            
            // LOCK FIFO 1 
            if(FIFO1SemValue[0] == 2 && filemandati[0] == 0){
              semOp(FIFO1SemId, 0, -2);
            //printf("\nWriting %i characters to FIFO1.\n",strlen(serialized));
              write(FIFO1, serialized, (strlen(serialized)) + 1);
              semOp(FIFO1SemId, 0, 1);
              filemandati[0] = 1;
              }
            
          getSemValues(FIFO2SemId, FIFO2SemValue);
            // LOCK FIFO 2
            if(FIFO2SemValue[0] == 2 && filemandati[1] == 0){
              semOp(FIFO2SemId, 0, -2);
            //printf("\nWriting %i characters to FIFO2.\n",strlen(&(serialized[1])));
              write(FIFO2, &(serialized[1]), strlen(&(serialized[1])) + 1);
              semOp(FIFO2SemId, 0, 1);
              filemandati[1] = 1;
              }

           getSemValues(MsgQueueSemId, MsgQueueSemValue);
          if(MsgQueueSemValue[0] != 0 && filemandati[2] == 0){
            // LOCK MSGQUEUE
            semOp(MsgQueueSemId, 0, -1);
            
            struct SerializedMessage msgqueueMsg = { 
              .mtype = 1,
            };
          
          memcpy(&(msgqueueMsg.mtext), &(serialized[2]), strlen(&(serialized[2])) + 1);     
           // strcpy(&(msgqueueMsg.mtext),&(serialized[2]));

          
        
          if(msgsnd(MsgQueueId,&(msgqueueMsg),mSize,0) == -1){
              ErrExit("<Client_N> msgqueue error");
            }
            filemandati[2] = 1;
            }

            
            getSemValues(ShmSemId, ShmSemValue);
            if(ShmSemValue[i % 50] == 2 && filemandati[3] == 0){
            // LOCK SHM
            semOp( ShmSemId, i % 50, -2);
            // Scrive il messaggio dalla Shared Memory
            // memcpy(&(shmDisposition->messages[i % 50]),&(serialized[3]),strlen(&(serialized[3])) + 1 );
            strcpy(&(shmDisposition->messages[i % 50]), &(serialized[3]));  

            semOp(ShmSemId, i % 50, 1);
            filemandati[3] = 1; 
            }
           }

         exit(code);

         }
    }

    //fflush(0);
    short values[2];
    
    do {
      sleep(0.1);
      getSemValues(CLIENTSemId, &values);  
    } while(values[0] > 0);

    semOp(CLIENTSemId,1,filePathsCounter);
  
    int status = 0;
    // get termination status of each created subprocess.
    while ((pid = wait(&status)) != -1)
        printf("Child %d Finished\n", pid);
    fflush(stdout);

    // Leggi il messaggio dalla MsgQueue
     struct SerializedMessage serializedMsg ;
    
    if(msgrcv(MsgQueueId, &serializedMsg, mSize, 0, -2)) {
      printf("Ricevuto %s\n", serializedMsg.mtext);  
    }
  
    // if ((msgRec = msgrcv(MsgQueueId, &serializedMsg, mSize, 1)) == -1){}
   
    // Close the FIFO
    // if (close(FIFO1) != 0 || close(FIFO2) != 0)
    // ErrExit("close failed");
}

void endComunication()
{
  
    printf("<client> received SIGUSR1. Client ends\n");
    exit(0);
}

void welcomeMessage()
{
    // Load the CWD
    char CWD[2000] = "";
    struct passwd *p = getpwuid(getuid());

    getcwd(CWD, sizeof(CWD));

    // https://pubs.opengroup.org/onlinepubs/007904975/basedefs/pwd.h.html
    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n", p->pw_name, CWD);
}

void blockSomeSignals()
{
    // set of signals
    sigset_t mySet;

    // initialize mySet to contain all signal
    sigfillset(&mySet);

    // remove SIGINT & SIGUSR1
    sigdelset(&mySet, SIGUSR1);
    sigdelset(&mySet, SIGINT);

    //  blocking all signals but SIGINT & SIgUSR1
    sigprocmask(SIG_SETMASK, &mySet, NULL);
}

void blockAllSignals()
{
    // Block all signals
    sigset_t mySet;
    sigfillset(&mySet);
    sigprocmask(SIG_SETMASK, &mySet, NULL);
}
