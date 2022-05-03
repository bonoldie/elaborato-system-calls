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
#include <fcntl.h>

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

    // Load the CWD to build the full file path
    char CWD[2000] = "";

    getcwd(CWD, sizeof(CWD));

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
            if (strstr(dentry->d_name, "sendme_") == dentry->d_name && fileStat.st_size < MAX_FILE_SIZE)
            {
                char fullFilePath[2000] = "";

                strcat(fullFilePath, CWD);
                strcat(fullFilePath, "/");
                strcat(fullFilePath, currFilePath);

                printf("<loadFilePaths> Found: %s (full path: %s)\n", dentry->d_name, fullFilePath);

                paths[pathsCounter] = malloc(sizeof(char) * (sizeof(fullFilePath) + 1));
                strcpy(paths[pathsCounter], fullFilePath);
                ++pathsCounter;
            }
        }
    }

    return pathsCounter;
}

int buildMessages(char *filePath, char **buff)
{
    int fd = open(filePath, O_RDONLY);
    char content[MAX_FILE_SIZE] = "";
    
    char metadata[MAX_FILE_SIZE] = "";
    sprintf(metadata,", %i, %s]", getpid(), filePath);

    int lenght = 0;

    if (fd < 0)
    {
        return -1;
    }

    while (read(fd, &(content[lenght]), 1) > 0)
        lenght++;


    for (int i = 0; i < 4; i++)
    {
        buff[i] = malloc(sizeof(char) * (MAX_FILE_SIZE / 4));
        strcat(buff[i], "[");
        strncat(buff[i], &(content[(lenght / 4) * i]), (lenght / 4));

        if(i == 3){
            strncat(buff[i], &(content[(lenght / 4) * i+1]), (lenght % 4));
        }
        strcat(buff[i],metadata);
    }

    close(fd);

    return lenght;
}