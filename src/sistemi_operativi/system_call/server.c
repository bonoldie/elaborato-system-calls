/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <error.h>

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h>

#include "err_exit.h"
#include "semaphore.h"
#include "fifo.h"
#include "shared_memory.h"
#include "fs.h"
#include "defines.h"

short FIFO1SemValues[1];
short FIFO2SemValues[1];
short ShmSemValues[50];

int MsgQueueId = -1;

struct MemoryDisposition *shmDisposition;

struct ApplicationMsg *messages;
int filesCounter = -1;
int messagesReceived = 0;

int main(int argc, char *argv[])
{

  setupSemaphores();

  //printSemValues(ShmSemId);
  //printSemValues(FIFO1SemId);
  //printSemValues(FIFO2SemId);
  //printSemValues(MsgQueueSemId);

  shmDisposition = init_shared_memory();
  shmDisposition->serverOk = 0;

  MsgQueueId = getMsgQueue(IPC_CREAT | S_IRUSR | S_IWUSR);

  printf("<Server> waiting for a client...\n");

  int FIFO1 = getFIFO(FIFO1PATH,O_RDWR);
  int FIFO2 = getFIFO(FIFO2PATH,O_RDWR);
 
  if (FIFO1 == -1 || FIFO2 == -1)
    ErrExit("fifo failed");

  while(1) {
    messagesReceived = 0;
  
  // reading bytes from fifo
  int bR = read(FIFO1, &filesCounter, sizeof(int));
  if (bR == -1)
    printf("<Server> it looks like the FIFO is broken");
  if (bR != sizeof(int) || bR == 0)
    printf("<Server> it looks like I did not receive");

  printf("<Server> Received %i \n", filesCounter);
  
  messages = (struct ApplicationMsg *)malloc(sizeof(struct ApplicationMsg) * filesCounter * 4);

  shmDisposition->serverOk = 1;

 while (messagesReceived != filesCounter * 4){
   
    size_t mSize = sizeof(struct SerializedMessage) - sizeof(long);
    char messageBuff[MESSAGE_SIZE] = "";
    int msgLength = 0;
    int msgRec = 0;

    memset(messageBuff,1,sizeof(char)*MESSAGE_SIZE);
   
    // Controllo FIFO1
    getSemValues(FIFO1SemId, FIFO1SemValues);
    // printSemValues(FIFO1SemId);// sem a 1
    // fflush(stdout);
   
    if (FIFO1SemValues[0] == 1){
      
      // Leggi il messaggio dalla FIFO1
      //printf("\nsono entrato in fifo1\n");
      
      while(read(FIFO1, &(messageBuff[msgLength]), sizeof(char)) > 0){
        printf("%c", messageBuff[msgLength]);
        if(messageBuff[msgLength] == '\0'){
          break;
        }
        
        msgLength++;
      }
      
      deserializeMessage(messageBuff, &(messages[messagesReceived]));
      messages[messagesReceived].medium = MEDIA_FIFO1_ID;

      semOp(FIFO1SemId, 0, 1); 
      messagesReceived++;
      msgLength = 0;
    }

    getSemValues(FIFO2SemId, FIFO2SemValues);
    // printSemValues(FIFO2SemId);
    // fflush(stdout);

   // Leggi il messaggio dalla FIFO2
    if (FIFO2SemValues[0] == 1){

      //printf("\nsono entrato in fifo2\n");
    
       while(read(FIFO2, &(messageBuff[msgLength]), sizeof(char)) > 0){
        printf("%c", messageBuff[msgLength]);
        if(messageBuff[msgLength] == '\0'){
          break;
        }
        
        msgLength++;
      }
      
      deserializeMessage(messageBuff, &(messages[messagesReceived]));
      messages[messagesReceived].medium = MEDIA_FIFO2_ID;

      semOp(FIFO2SemId, 0, 1); 
      messagesReceived++;
      msgLength = 0;
    }
  
   // Leggi il messaggio dalla MsgQueue
   struct SerializedMessage serializedMsg ;
   

    if ((msgRec = msgrcv(MsgQueueId, &serializedMsg, mSize, 1, IPC_NOWAIT)) == -1){
      if (errno != EAGAIN && errno != ENOMSG)
        ErrExit("<Server> Error while reading from MsgQueue");
    }
   
    if (msgRec > 0){
      //printf("\nSono nella msq\n");
      deserializeMessage(&(serializedMsg.mtext), &(messages[messagesReceived]));
      messages[messagesReceived].medium = MEDIA_MSGQUEUE_ID;

      semOp(MsgQueueSemId, 0, 1);
      messagesReceived++;
      msgLength = 0;
    }
  
    getSemValues(ShmSemId, ShmSemValues);
 
    for (int sem = 0; sem < 50; sem++)
    {
      
      if (ShmSemValues[sem] == 1)
      {
        // Leggi il messaggio dalla Shared Memory
        //printf("\nSono nella shmem\n");
        //memcpy(messageBuff, &(shmDisposition->messages[sem]), MESSAGE_SIZE);
        strcpy(messageBuff,&(shmDisposition->messages[sem]));
        printf("SHARED: %s\n",messageBuff);
        deserializeMessage(messageBuff, &(messages[messagesReceived]));
        messages[messagesReceived].medium = MEDIA_SHM_ID;
        semOp(ShmSemId, sem, 1);
        messagesReceived++;
        msgLength = 0;
      }
    }
  }
  
  // Ordina tutti i messaggi ricevuti
  sortMessages(messages,messagesReceived);

  // Scrittura su file
  for(int i = 0; i < messagesReceived; i += 4){
    writeOutFile(&(messages[i]));
  }

  free(messages);

  //printf("HO SCRITTO");
  fflush(stdout);

    struct SerializedMessage msgqueueMsg = { 
      .mtype = 2,
      .mtext = "FINITO",
    };
    
    size_t mSize = sizeof(struct SerializedMessage) - sizeof(long);
    int msgCode = 0;

    if((msgCode = msgsnd(MsgQueueId,&(msgqueueMsg),mSize,0)) == -1){
      ErrExit("<Client_N> msgqueue error");
    }

  printf("\n Sending msgqueue message to client(%i)\n", msgCode);

    }
  return 0;
}