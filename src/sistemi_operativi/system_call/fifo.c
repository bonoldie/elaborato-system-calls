/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "err_exit.h"
#include "defines.h"
#include "fifo.h"

int getFIFO1(mode_t mode)
{
    int fifoRes = mkfifo(FIFO1PATH, S_IRUSR | S_IWUSR);

    if (fifoRes < 0 && errno != EEXIST)
    {
        ErrExit('Cannot create fifo 1');
    }

    return open(FIFO1PATH, mode);
}

int getFIFO2(mode_t mode)
{
    int fifoRes = mkfifo(FIFO2PATH, S_IRUSR | S_IWUSR);

    if (fifoRes < 0 && errno != EEXIST)
    {
        ErrExit('Cannot create fifo 2');
    }

    return open(FIFO2PATH, mode);
}
