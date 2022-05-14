#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"

const char *fileName = "myfile1";

int main (int argc, char *argv[]) {

    // check command line input arguments
    if (argc <= 1) {
        printf("Usage: %s cmd [arguments]\n", argv[0]);
        return 0;
    }

    switch (fork()) {
        case -1: {
            errExit("fork failed");
        }
        case 0: {
            // close the standard output and error stream
            close(STDOUT_FILENO); // close file descriptor 1
            close(STDERR_FILENO); // close file descriptor 2

            // create a new file. The value returned by open will be 1 as it is
            // lowest available index
            int fd = open(fileName,
                          O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

            if (fd == -1)
                errExit("open failed");

            // clone the file descriptor 1. The value returned by dup will be 2
            // as it is  lowest available index
            dup(fd);

            // both file descriptors 1 and 2 are pointing to the created file now.

            // replace the current process image with a new process image
            execvp(argv[1], &argv[1]);
            errExit("execvp failed");
        }
        default: {
            int status;
            // wait the termination of the child process
            if (wait(&status) == -1)
                errExit("wait failed");

            printf("Command %s exited with status %d\n", argv[1], WEXITSTATUS(status));
        }
    }

    return 0;
}
