/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>

#include <signal.h>

#include <unistd.h>
#include <errno.h>

#include <pwd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include "err_exit.h"
#include "fs.h"
#include "defines.h"
#include "shared_memory.h"
#include "fifo.h"

struct MemoryDisposition *shmDisposition;

void startComunication();
void endComunication();

void welcomeMessage();

void blockAllSignals();
void blockSomeSignals();

char CWDARG[250];

char *filePaths[MAX_FILES];
int filePathsCounter;

struct ApplicationMsg messages[4];

void sigHandler(int sig)
{

    if (sig == SIGINT)
    {
        startComunication();
    }
    else if (sig == SIGUSR1)
    {
        printf("<client> received SIGUSR1. Client ends\n");
        endComunication();
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    blockSomeSignals();

    // imposto directory corrente
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

    while (1)
    {

        sleep(10);
    }

    return 0;
}

void startComunication()
{
    blockAllSignals();

    // imposto CWD
    chdir(CWDARG);

    welcomeMessage();

    filePathsCounter = loadFilePaths(".", filePaths);

    // FIFO 1 CLIENT
    printf("<Client> Opening FIFO1 ...\n");
    // Open the FIFO in write-only mode
    int FIFO1 = getFIFO1(O_WRONLY);

    if (FIFO1 == -1)
        ErrExit("open failed");

    printf("<Client> sending %i\n", filePathsCounter);
    // Wrinte  two integers to the opened FIFO
    if (write(FIFO1, &filePathsCounter, sizeof(int)) <= 0)
        ErrExit("write failed");

    while (shmDisposition->serverOk == 0)
    {
        printf("<Client> Waiting for serverOk");
        sleep(1);
    }

    shmDisposition->serverOk = 0;

    for (int i = 0; i < filePathsCounter; ++i)
    {
        buildMessages(filePaths[i], messages);

        for (int j = 0; j < 4; ++j)
        {
            char serialized[MESSAGE_SIZE] = "";
            serializeMessage(&(messages[j]), serialized);
            printf("%s \n", serialized);
        }
    }

    printf("<Client> Received serverOk");

    fflush(stdout);

    // Close the FIFO
    if (close(FIFO1) != 0)
        ErrExit("close failed");
}

void endComunication()
{
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