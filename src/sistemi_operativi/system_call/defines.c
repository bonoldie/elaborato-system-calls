/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <sys/stat.h>

#include "defines.h"
#include "err_exit.h"

char MEDIA_SHM [] = "ShdMem"; 
char MEDIA_FIFO1 [] = "FIFO1"; 
char MEDIA_FIFO2 [] = "FIFO2"; 
char MEDIA_MSGQUEUE [] = "MsgQueue";

int getMsgQueue(mode_t mode){
  // get the message queue identifier
  int msqid = msgget(MSGQUEUE_KEY, S_IRUSR | mode);
  if (msqid == -1)
    ErrExit("msgget failed");
  return msqid;
}

int sortMessages(struct ApplicationMsg *msgs, int arrayLength)
{
  for (int i = 0; i < arrayLength - 1; ++i)
  {
    for (int j = i; j < arrayLength - 1; ++j)
    {
      if (msgs[i].PID > msgs[j].PID)
      {
        msgs[i], msgs[j] = msgs[j], msgs[i];
      }
    }
  }
}
