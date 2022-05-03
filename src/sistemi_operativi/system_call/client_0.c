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

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include "err_exit.h"
#include "fs.h"
#include "defines.h"

void startComunication();
void endComunication();

char CWD[250];

char *filePaths[MAX_FILES];
int filePathsCounter;

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
    // set of signals
    sigset_t mySet;
    // initialize mySet to contain all signal
    sigfillset(&mySet);
    // remove SIGINT & SIGUSR1
    sigdelset(&mySet, SIGUSR1);
    sigdelset(&mySet, SIGINT);
    // sigdelset(&mySet, SIGTERM);
    //  blocking all signals but SIGINT & SIgUSR1
    sigprocmask(SIG_SETMASK, &mySet, NULL);

    // imposto directory corrente
    if (argc != 2)
    {
        printf("Usage: \n");
        printf("%s <directory> \n", argv[0]);
        return 1;
    }

    strcat(CWD, argv[1]);

    // set the function sigHandler for signals
    if (signal(SIGINT, sigHandler) == SIG_ERR ||
        signal(SIGUSR1, sigHandler) == SIG_ERR)
    {
        ErrExit("change signal handler failed");
    }

    while (1)
    {
        sleep(10);
    }

    return 0;
}

void startComunication()
{
    // blocco tutti i segnali
    sigset_t mySet;
    sigfillset(&mySet);
    sigprocmask(SIG_SETMASK, &mySet, NULL);

    // imposto directory corrente
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  chdir(CWD);
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // https://pubs.opengroup.org/onlinepubs/007904975/basedefs/pwd.h.html
    struct passwd *p = getpwuid(getuid());
    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n", p->pw_name, CWD);

    filePathsCounter = loadFilePaths(CWD, filePaths);
}

void endComunication()
{
}
