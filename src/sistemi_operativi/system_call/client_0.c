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
#include <sys/sem.h>

#include <dirent.h>

#include "err_exit.h"
#include "fs.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

struct MemoryDisposition *shmDisposition;

void startComunication();
void endComunication();

void welcomeMessage();

void blockAllSignals();
void blockSomeSignals();

short FIFO1SemVelue[1];
short FIFO2SemValue[1];
short ShmSemValues[50];

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
        endComunication();
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

    setupSemaphores();

    while (1)
    {

        sleep(10);
    }

    return 0;
}

void startComunication()
{
    blockAllSignals();
    initSemaphores();

    // imposto CWD
    chdir(CWDARG);

    welcomeMessage();

    filePathsCounter = loadFilePaths(".", filePaths);

    initClientSemaphore(filePathsCounter);

    // FIFO 1 CLIENT
    printf("<Client> Opening FIFO1 ...\n");
    // Open the FIFO in write-only mode
    int FIFO1 = getFIFO1(O_WRONLY);

    printf("<Client> Waiting for serverOk\n");

    printf("<Client> sending %i\n", filePathsCounter);
    // Wrinte  two integers to the opened FIFO
    if (write(FIFO1, &filePathsCounter, sizeof(int)) <= 0)
        ErrExit("write failed");

    while (shmDisposition->serverOk == 0)
    {
        sleep(1);
    }

    shmDisposition->serverOk = 0;

    printf("<Client> Received serverOk\n");

    fflush(stdout);

    // Valore sem client N dei path
    printSemValues(CLIENTSemId, filePathsCounter);

    // Creazione dei figli
    pid_t pid;

    for (int i = 0; i < filePathsCounter; ++i)
    {

        int code = (int)(((double)rand() / RAND_MAX) * 255);
        pid = fork();
        if (pid == -1)
        {
            printf("Child %d not created!", i);
        }
        else if (pid == 0)
        {
            // wait the i-th child

            //!!!!!!!!!
            buildMessages(filePaths[i], messages);

            for (int j = 0; j < 4; ++j)
            {
                char serialized[MESSAGE_SIZE] = "";
                serializeMessage(&(messages[j]), serialized);
                printf("%s \n", serialized);
            }
            printf("PIDchild: %d, PIDparent: %d\n", getpid(), getppid());
            semOp(CLIENTSemId, i, -1);
            semOp(CLIENTSemId, (unsigned short)(i == 0) ? filePathsCounter - 1 : i - 1, 1);
            exit(code);
        }
    }

    int status = 0;
    // get termination status of each created subprocess.
    while ((pid = wait(&status)) != -1)
        printf("Child %d Finished\n", pid);

    fflush(stdout);

    // Close the FIFO
    if (close(FIFO1) != 0)
        ErrExit("close failed");
}

void endComunication()
{
    printf("<client> received SIGUSR1. Client ends\n");
    exit(0);
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
