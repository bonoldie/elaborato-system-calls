/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <sys/stat.h>

#include "defines.h"
#include "err_exit.h"

char * MEDIA []= {
  "FIFO1",
  "FIFO2",
  "MsgQueue",
  "ShdMem"
}; 

int getMsgQueue(mode_t mode){
  // get the message queue identifier
  int msqid = msgget(MSGQUEUE_KEY, S_IRUSR | mode);
  if (msqid == -1)
    ErrExit("msgget failed");
  return msqid;
}

int sortMessages(struct ApplicationMsg *msgs, int arrayLength)
{  
  for (int  i = 0; i < arrayLength - 1; i++) {
      int index = i;
      for (int j = i + 1; j < arrayLength; j++)
         if (msgs[j].PID < msgs[index].PID) 
            index = j;
      
      struct ApplicationMsg temp = msgs[index];  
      msgs[index] = msgs[i];
      msgs[i] = temp;
  
    }


  for (int  globalIndex = 4; globalIndex < arrayLength - 1; globalIndex += 4) {
    
  for (int  i = globalIndex; i < globalIndex + 4; i++) {
      int index = i;
      for (int j = i + 1; j < globalIndex + 4; j++)
         if (msgs[j].medium < msgs[index].medium) 
            index = j;
      
      struct ApplicationMsg temp = msgs[index];  
      msgs[index] = msgs[i];
      msgs[i] = temp;
  
    }
  }
    

    }


