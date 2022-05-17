/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "err_exit.h"
#include "defines.h"
#include "fifo.h"

int getFIFO(char * path, mode_t mode)
{
    int fifoRes = mkfifo(path, S_IRUSR | S_IWUSR);

    if (fifoRes == -1 && errno != EEXIST)
    {
        ErrExit('Cannot create fifo 1');
    }

    return open(path, mode);
}



