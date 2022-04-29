/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "err_exit.h"

//void startComunication();
//void endComunication();

void sigHandler(int sig){
  if(sig == SIGINT) { 
    printf("The signal %s was caught!\n", strsignal(sig));
    
  } else if (sig == SIGUSR1) {
    printf("<client> received SIGUSR1. Client ends\n");
    if (kill(getpid(), SIGKILL) == -1){
      ErrExit("kill failed");
    }
  }
}
  
int main(int argc, char * argv[]) {
  // set of signals
  sigset_t mySet;
  // initialize mySet to contain all signal
  sigfillset(&mySet);
  // remove SIGINT & SIGUSR1
  sigdelset(&mySet, SIGUSR1);
  sigdelset(&mySet, SIGINT);
  // blocking all signals but SIGINT & SIgUSR1
  sigprocmask(SIG_SETMASK, &mySet, NULL);
  
  // set the function sigHandler for signals
  if (signal(SIGINT, sigHandler) == SIG_ERR ||
      signal(SIGUSR1, sigHandler) == SIG_ERR){
      ErrExit("change signal handler failed");
  }
  
while(1);
  // elenco dei file (find??)
  
  return 0;
}
