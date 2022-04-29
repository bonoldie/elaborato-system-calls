#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <stdio.h>

#include "checkMethods.h"

int checkPermissions(char *pathname, mode_t st_mode) {
    if (pathname == NULL)
        return 0;

    struct stat statbuf;
    if (stat(pathname, &statbuf) == -1)
        return 0;

    // getting file permissions only for
    // the current user:
    //  user    group   other
    // |r|w|x| |r|w|x| |r|w|x| &  (all permissions)
    // |1|1|1| |0|0|0| |0|0|0| =  (mask to only get user permissions)
    //--------------------------
    // |r|w|x| |0|0|0| |0|0|0|    (== st_mode ?)
    
    return (statbuf.st_mode & (7 << 6)) == st_mode;
}

int checkFileSize(char *pathname, off_t size) {
    if (pathname == NULL)
        return 0;

    struct stat statbuf;
    if (stat(pathname, &statbuf) == -1)
        return 0;

    return statbuf.st_size >= size;
}

int checkFileName(char *filename1, char *filename2) {
    return (filename1 == NULL || filename2 == NULL)? 0
           : strcmp(filename1, filename2) == 0;
}
