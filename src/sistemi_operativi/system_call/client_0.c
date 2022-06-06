// @file client.c
// @brief Contiene l'implementazione del client.

// kill -SIGKILL $(ps -aux | grep -m1 client_0 | cut -d' ' -f6) $(ps -aux | grep -m1 client_0 | cut -d' ' -f7)
// rm -rfv /src/sistemi_operativi/system_call/manda

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

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

char CWDARG[2000];

char *filePaths[MAX_FILES];
int filePathsCounter;

int MsgQueueId = -1;

// Main sigHandler
void sigHandler(int sig)
{
  if (sig == SIGINT)
  {
    // blocks all signals
    blockAllSignals();

    startComunication();

    // blocks all signals besides SIGINT and SIGUSR1
    blockSomeSignals();
  }
  else if (sig == SIGUSR1)
  {
    endComunication();
  }
}

int main(int argc, char *argv[])
{
  // checks for sendme base path
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

  // removed
  while (1)
    sleep(1000000);

  return 0;
}

void startComunication()
{
  // reset all the global variables
  filePathsCounter = 0;
  int filemandati[4] = {0, 0, 0, 0};
  short values[2];
  int status = 0;

  // imposto CWD
  chdir(CWDARG);

  // print the welcome message
  welcomeMessage();

  // loads all the valid sendme files from the CWD
  loadFilePaths(".", filePaths, &filePathsCounter);

  // reset all the semaphores
  initSemaphores();
  initClientSemaphore(filePathsCounter);

  // Opening the 4 IPC channels

  // Open the FIFO in write-only mode
  int FIFO1 = getFIFO(FIFO1PATH, O_RDWR);
  int FIFO2 = getFIFO(FIFO2PATH, O_RDWR);

  // Create MSGQUEUE
  MsgQueueId = getMsgQueue(IPC_CREAT | S_IRUSR | S_IWUSR);

  size_t mSize = sizeof(struct SerializedMessage) - sizeof(long);

  printf("<Client> Waiting for serverOk\n");

  printf("<Client> sending %i\n", filePathsCounter);

  // Writing sendme files count to server via FIFO1
  if (write(FIFO1, &filePathsCounter, sizeof(int)) <= 0)
    ErrExit("write failed");

  // waiting for server confirmation via Shared memory
  while (shmDisposition->serverOk == 0)
  {
    sleep(1);
  }

  // Resets sever confirmation flag
  shmDisposition->serverOk = 0;

  printf("<Client> Received serverOk\n");

  fflush(stdout);

  // Creazione dei figli
  pid_t pid;

  for (int i = 0; i < filePathsCounter; ++i)
  {

    pid = fork();

    if (pid == -1)
    {
      printf("Child %d not created!", i);
    }
    else if (pid == 0)
    {
      // Messages to send via IPCs
      struct ApplicationMsg messages[4];
      
      // Serialized version of messages
      char serialized[4][MESSAGE_SIZE];

      // Loads a file from the storage to the ApplicationMsg array
      buildMessages(filePaths[i], messages);

      for (int j = 0; j < 4; ++j)
      {
        // Serialize each message 
        serializeMessage(&(messages[j]), &(serialized[j]));
      }

      // Wait for every child to start sending messages
      // sem da 50 a 0
      semOp(CLIENTSemId, 0, -1);
      // sem da 0 a 50
      semOp(CLIENTSemId, 1, -1);


      while (filemandati[0] == 0 || filemandati[1] == 0 || filemandati[2] == 0 || filemandati[3] == 0)
      {

        getSemValues(FIFO1SemId, FIFO1SemValue);
        // LOCK FIFO 1
        if (FIFO1SemValue[0] == 2 && filemandati[0] == 0)
        {
          semOp(FIFO1SemId, 0, -2);

          write(FIFO1, serialized, (strlen(serialized)) + 1);
          semOp(FIFO1SemId, 0, 1);
          filemandati[0] = 1;
        }

        getSemValues(FIFO2SemId, FIFO2SemValue);
        // LOCK FIFO 2
        if (FIFO2SemValue[0] == 2 && filemandati[1] == 0)
        {
          semOp(FIFO2SemId, 0, -2);

          write(FIFO2, &(serialized[1]), strlen(&(serialized[1])) + 1);

          semOp(FIFO2SemId, 0, 1);
          filemandati[1] = 1;
        }

        getSemValues(MsgQueueSemId, MsgQueueSemValue);

        if (MsgQueueSemValue[0] != 0 && filemandati[2] == 0)
        {
          // LOCK MSGQUEUE
          semOp(MsgQueueSemId, 0, -1);

          struct SerializedMessage msgqueueMsg = {
              .mtype = 1,
          };

          memcpy(&(msgqueueMsg.mtext), &(serialized[2]), strlen(&(serialized[2])) + 1);

          if (msgsnd(MsgQueueId, &(msgqueueMsg), mSize, 0) == -1)
          {
            ErrExit("<Client_N> msgqueue error");
          }
          filemandati[2] = 1;
        }

        getSemValues(ShmSemId, ShmSemValue);
        if (ShmSemValue[i % 50] == 2 && filemandati[3] == 0)
        {
          // LOCK SHM
          semOp(ShmSemId, i % 50, -2);

          strcpy(&(shmDisposition->messages[i % 50]), &(serialized[3]));

          semOp(ShmSemId, i % 50, 1);
          filemandati[3] = 1;
        }
      }

      exit(0);
    }
  }

  // Client_0 wait for all children to load and serialize messages 
  do
  {
    sleep(0.5);
    getSemValues(CLIENTSemId, &values);
  } while (values[0] > 0);

  // release child lock to start them all together
  semOp(CLIENTSemId, 1, filePathsCounter);

  // get termination status of each created subprocess.
  while ((pid = wait(&status)) != -1) printf("<Client> Child %d Finished\n", pid);
  
  // Read the message from the Message queue
  struct SerializedMessage serializedMsg;

  if (msgrcv(MsgQueueId, &serializedMsg, mSize, 0, -2))
  {
    printf("<Client> Received %s from MsgQueue\n", serializedMsg.mtext);
  }
}

void endComunication()
{
  printf("<Client> received SIGUSR1. Client ends\n");
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
