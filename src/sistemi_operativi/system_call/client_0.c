/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

int main(int argc, char * argv[]) {
  sigset_t mySet, prevSet;

  sigfillset(&mySet);

  sigdelset(&mySet, SIGUSR1);
  sigdelset(&mySet, SIGINT);

  sigprocmask(SIG_SETMASK, &mySet, &prevSet);

    return 0;
}
