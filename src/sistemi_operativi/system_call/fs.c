/// @file fs.c
/// @brief Contiene la definizioni di funzioni specifiche per la gestione del FS.

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
#include "defines.h"

#include "fs.h"

int loadFilePaths(char *currdir, char **paths)
{
    DIR *dirp = opendir(currdir);
    struct dirent *dentry;

    // Current directory valid files counter
    int pathsCounter = 0;

    if (dirp == NULL)
        return;

    printf("<loadFilePaths> Inspecting: %s\n", currdir);

    // interate until NULL is returned as result
    while ((dentry = readdir(dirp)) != NULL)
    {
        char currFilePath[2000] = "";

        struct stat fileStat;

        // Calculate the current file path
        strcat(currFilePath, currdir);
        strcat(currFilePath, "/");
        strcat(currFilePath, dentry->d_name);

        // Retrieve file meta
        stat(currFilePath, &fileStat);

        // Skip the current and previous directories
        if (strcmp(dentry->d_name, ".") == 0 ||
            strcmp(dentry->d_name, "..") == 0)
        {
            continue;
        }

        // https://man7.org/linux/man-pages/man7/inode.7.html
        if ((fileStat.st_mode & S_IFMT) == S_IFDIR)
        {
            pathsCounter += loadFilePaths(currFilePath, paths);
        }
        else if ((fileStat.st_mode & S_IFMT) == S_IFREG)
        {
            // Match the file requirements
            if (strstr(dentry->d_name, "sendme_") == dentry->d_name && fileStat.st_size < 4000)
            {
            
                printf("<loadFilePaths> Found: %s (full path: %s)\n", dentry->d_name, currFilePath);

                paths[pathsCounter] = malloc(sizeof(char) * 2000);
                strcpy(paths[pathsCounter], currFilePath);
                ++pathsCounter;
            }
        }
    }

    return pathsCounter;
}