/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"



int main(int argc, char * argv[]) {

  /*int size = 50;

  if(mkfifo("/src/sistemi_operativi", S_IRUSR|S_IWUSR) == -1)
    errExit("non sono riuscito a creare la fifo1");
    
 // if(mkfifo("/src/sistemi_operativi", S_IRUSR|S_IWUSR) == -1)
  //  errExit("non sono riuscito a creare la fifo2");
  //non so se per fare una seconda fifo devo mettere un pathname diverso dalla fifo1
  // sigset_t ??

  int shdm = shmget(IPC_PRIVATE, size, IPC_CREAT | S_IRUSR | S_IWUSR);

  int msgq = msgget(IPC_PRIVATE, IPC_CREAT | S_IRUSR | S_IWUSR);

  int sem = semget(PIC_PRIVATE, 5,  IPC_CREAT | S_IRUSR | S_IWUSR);

  int fd = open("/src/sistemi_operativi", O_RDONLY);
*/
  
    return 0;
}
