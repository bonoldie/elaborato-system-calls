/// @file client.c
/// @brief Contiene l'implementazione del client.
/// https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md
//  kill -SIGKILL $(ps -aux | grep -m1 client_0 | cut -d' ' -f6)
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "err_exit.h"

#define MAX_FILES 100

void startComunication();
void endComunication();
void loadFilePaths();

char CWD [250];

char* filePaths[MAX_FILES];
int filePathsCounter;

void sigHandler(int sig){
  
  if(sig == SIGINT) { 
    startComunication();
  } else if (sig == SIGUSR1) {
    printf("<client> received SIGUSR1. Client ends\n");
    endComunication();
    exit(0);
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
  //sigdelset(&mySet, SIGTERM);
  // blocking all signals but SIGINT & SIgUSR1
  sigprocmask(SIG_SETMASK, &mySet, NULL);

  //imposto directory corrente
  if (argc != 2) {
      printf("Usage: \n");
      printf("%s <directory> \n", argv[0]);
      return 1;
  }
  
  strcat(CWD,argv[1]);
  
  // set the function sigHandler for signals
  if (signal(SIGINT, sigHandler) == SIG_ERR ||
      signal(SIGUSR1, sigHandler) == SIG_ERR){
      ErrExit("change signal handler failed");
  }

  while(1){
     sleep(10);
  }
  
  return 0;
}

void startComunication(){
  //blocco tutti i segnali
  sigset_t mySet;
  sigfillset(&mySet);
  sigprocmask(SIG_SETMASK, &mySet, NULL);

  //imposto directory corrente
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // chdir(CWD);
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  

  // https://pubs.opengroup.org/onlinepubs/007904975/basedefs/pwd.h.html
  struct passwd *p = getpwuid(getuid());
  printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n", p->pw_name, CWD); 

  filePathsCounter = 0;

  
  loadFilePaths(CWD);

  for(int i = 0 ;  i < filePathsCounter; ++i){
    printf("%s \n", filePaths[i]);
  }
}

void loadFilePaths(char *currdir){
  DIR *dirp = opendir(currdir);
  struct dirent * dentry;
  
  if (dirp == NULL) return;

  
    //interate until NULL is returned as result
  while ((dentry = readdir(dirp)) != NULL) {
    char * currFilePath = malloc(sizeof(char)*2000); 
    *currFilePath = '\0';

    struct stat fileStat;

    // Calculate the current file path
    strcat(currFilePath,currdir);
    strcat(currFilePath,"/");
    strcat(currFilePath,dentry->d_name);

    // Retrieve file meta
    stat(currFilePath, &fileStat);
    
    if (strcmp(dentry->d_name, ".") == 0 ||
          strcmp(dentry->d_name, "..") == 0)
     {
      continue;
    }
    
    // https://man7.org/linux/man-pages/man7/inode.7.html
    if((fileStat.st_mode & S_IFMT) == S_IFDIR){
      loadFilePaths(currFilePath);
    } else if((fileStat.st_mode & S_IFMT) == S_IFREG) {
      if(strstr(dentry->d_name,"sendme_") == dentry->d_name && fileStat.st_size < 4000){
        // Current file name: currdir + dentry->d_name
        filePaths[filePathsCounter]  = malloc(sizeof(char)*2000);
        strcpy(filePaths[filePathsCounter], currFilePath);
        ++filePathsCounter;
      }
    }
  }
}

void endComunication() {
  
}

