/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <sys/stat.h>

#include "defines.h"
#include "err_exit.h"

int getMsgQueue(mode_t mode){
  // get the message queue identifier
    int msqid = msgget(MSGQUEUE_KEY, S_IRUSR | mode);
    if (msqid == -1)
        ErrExit("msgget failed");
    return msqid;
}