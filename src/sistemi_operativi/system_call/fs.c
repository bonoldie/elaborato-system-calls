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

int buildMessages(char *filePath, struct ApplicationMsg *msgs)
{
    int fd = open(filePath, O_RDONLY);
    char content[MAX_FILE_SIZE] = "";
    int lenght = 0;

    if (fd < 0)
    {
        return -1;
    }


    while (read(fd, &(content[lenght]), 1) > 0)
        lenght++;

    for (int i = 0; i < 4; i++)
    {
        
        char payload[MESSAGE_PAYLOAD_SIZE] = "";

        strncat(payload, &(content[(lenght / 4) * i]), (lenght / 4));

        if (i == 3)
        {
            strncat(payload, &(content[(lenght / 4) * i + 1]), (lenght % 4));
        }

        strcpy(&(msgs[i].payload),payload);
        strcpy(&(msgs[i].path),filePath);
        msgs[i].PID = getpid();

        printf ("<buildMessages> payload<%s> PID<%i> path<%s> \n", msgs[i].payload,msgs[i].PID,msgs[i].path);
    }

    close(fd);
    return lenght;
}

int serializeMessage(struct ApplicationMsg *msg, char *buff)
{
    buff[0] = '\0';
    char PID[10] = ""; 
    sprintf(PID, "%i", msg->PID);

    strcat(buff, "[");
    strcat(buff, msg->payload);
    strcat(buff, ", ");
    strcat(buff, PID);
    strcat(buff, ", ");
    strcat(buff, msg->path);
    strcat(buff, "]");

    return 0;
}

int deserializeMessage(char *buff, struct ApplicationMsg *msg)
{
    char PID[10] = ""; 

    char *lastBracket = strstr(buff,"]");
    char *firstComma = strstr(buff,", ");
    char *secondComma = strstr(buff[(int)(firstComma+1)],", ");
    

    if(lastBracket == NULL || firstComma == NULL || secondComma == NULL){
        return -1;
    }

    strncat(msg->payload,buff[1], ((firstComma - buff)/sizeof(char)) + 1);

    strncat(PID,buff[(int)(firstComma + 2*sizeof(char))], ((secondComma - buff)/sizeof(char)) + 1);
    msg->PID = atoi(PID);

    strncat(msg->path,buff[(int)(secondComma + 2)], (lastBracket - secondComma)/sizeof(char));

    return 0;
}
