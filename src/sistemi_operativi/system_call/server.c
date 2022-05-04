/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>



#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "err_exit.h"
#include "fifo.h"
#include "shared_memory.h"
#include "semaphore.h"

struct MemoryDisposition *shmDisposition;

int main(int argc, char * argv[]) {

  shmDisposition = init_shared_memory();
  shmDisposition->serverOk = 0;
  
  char *fpath = "/tmp/FIFO1";
  
  if(mkfifo(fpath, S_IRUSR|S_IWUSR) == -1)
      ErrExit("non sono riuscito a creare la fifo1");

  printf("<Server> FIFO %s created!\n", fpath);

  printf("<Server> waiting for a client...\n");
  int FIFO1 = open(fpath, O_RDONLY);
    if(FIFO1 == -1)
      ErrExit("fifo failed");
 
  int v;
  //reading bytes from fifo
  int bR = read(FIFO1, &v, sizeof(int));
  if (bR == -1)
        printf("<Server> it looks like the FIFO is broken");
    if (bR != sizeof(int) || bR == 0)
        printf("<Server> it looks like I did not receive");
    else
        printf("<Server> %i \n", v);

  shmDisposition->serverOk = 1;
  
    
  // request->messages[0];
  // request->messages[1];
  // ...
  
  // Close the FIFO
    if (close(FIFO1) != 0)
        ErrExit("close failed");

    printf("<Server> removing FIFO...\n");
    // Remove the FIFO
    if (unlink(fpath) != 0)
        ErrExit("unlink failed");




  
   /*if(mkfifo(fpath, S_IRUSR|S_IWUSR) == -1)
      ErrExit("non sono riuscito a creare la fifo2");*/
  //non so se per fare una seconda fifo devo mettere un pathname diverso dalla fifo1
  // sigset_t ??

  //cambia IPC_PRIVATE per avere key scelte da noi
 /* int shdm = shmget(IPC_PRIVATE, size, IPC_CREAT | S_IRUSR | S_IWUSR);

  int msgq = msgget(IPC_PRIVATE, IPC_CREAT | S_IRUSR | S_IWUSR);

  int sem = semget(PIC_PRIVATE, 5,  IPC_CREAT | S_IRUSR | S_IWUSR);*/
  
    return 0;
}