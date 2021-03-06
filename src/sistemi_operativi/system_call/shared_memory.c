/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
/// specifiche per la gestione della MEMORIA CONDIVISA.

#include "err_exit.h"
#include "shared_memory.h"
#include "defines.h"

#include <sys/shm.h>
#include <sys/stat.h>

 int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        ErrExit("shmget failed");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *ptr_sh = shmat(shmid, 0, shmflg);
    if (ptr_sh == (void *)-1)
        ErrExit("shmat failed");

    return ptr_sh;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, 0) == -1)
        ErrExit("shmctl failed");
}

struct MemoryDisposition * init_shared_memory(){
  
  // allocate a shared memory segment
    printf("Allocating a shared memory segment...\n");
    int shmidServer = alloc_shared_memory(SHM_KEY, sizeof(struct MemoryDisposition));

  // attach the shared memory segment
    printf("Attaching the shared memory segment...\n");
    struct MemoryDisposition *shmDisposition = (struct MemoryDisposition*)get_shared_memory(shmidServer, 0);

  return shmDisposition;
}