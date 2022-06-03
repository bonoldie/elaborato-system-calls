/// @file fs.c
/// @brief Contiene la definizioni di funzioni specifiche per la gestione del FS.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

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

int loadFilePaths(char *currdir, char **paths,int *pathsCounter)
{
    DIR *dirp = opendir(currdir);
    struct dirent *dentry;

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
            loadFilePaths(currFilePath, paths, pathsCounter);
        }
        else if ((fileStat.st_mode & S_IFMT) == S_IFREG)
        {
            // Match the file requirements
            if (strstr(dentry->d_name, "sendme_") == dentry->d_name && fileStat.st_size < MAX_FILE_SIZE)
            {
                char fullFilePath[4096] = "";

                strcat(fullFilePath, CWD);
                strcat(fullFilePath, "/");
                strcat(fullFilePath, currFilePath);

                strcpy(&fullFilePath, realpath(fullFilePath, NULL));

                printf("<loadFilePaths> #%i Found: %s (full path: %s)\n", *pathsCounter,  dentry->d_name, fullFilePath);
                paths[*pathsCounter] = malloc(strlen(fullFilePath) + 1);
                strcpy(paths[*pathsCounter], fullFilePath);
                *pathsCounter += 1;
            }
        }
    }

    return 0;
}

int buildMessages(char *filePath, struct ApplicationMsg *msgs)
{
    int fd = open(filePath, O_RDONLY);
    char content[MAX_FILE_SIZE] = "";
    int lenght = 0;

   printf("<buildMessages> loading PATH %s PID<%i> \n", filePath, getpid());

    if (fd < 0)
    {
      printf("<buildMessages> open failed for PATH %s PID<%i> \n", filePath, getpid());

        return -1;
    }

    while (read(fd, &(content[lenght]), 1) > 0)
        lenght++;


    int startIndex = 0;
  
    for (int i = 0; i < 4; i++)
    {

        char payload[MESSAGE_PAYLOAD_SIZE] = "";

        strncat(payload, &(content[startIndex]), (lenght / 4));
        startIndex += (lenght / 4);

        if (lenght % 4 >= i + 1)
        {
          strncat(payload, &(content[startIndex]), 1);
          startIndex += 1;
        }

        strcpy(&(msgs[i].payload), payload);
        strcpy(&(msgs[i].path), filePath);
        msgs[i].PID = getpid();

        printf("<buildMessages> payload<%s> PID<%i> path<%s> \n", msgs[i].payload, msgs[i].PID, msgs[i].path);
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
    //printf("CCCC");
  
    char PID[10] = "";

    char *lastBracket = strstr(buff, "]");
    char *firstComma = strstr(buff, ", ");
    char *secondComma = strstr(firstComma + 1, ", ");

    if (lastBracket == NULL || firstComma == NULL || secondComma == NULL)
    {
        return -1;
    }

    //printf("\n %x %x %x %x\n",buff, firstComma,secondComma,lastBracket);
  
    strncat(msg->payload, &(buff[1]), firstComma - buff - 1);
    printf("%s\n",msg->payload);
    
    strncat(PID, firstComma + 2 , secondComma - firstComma - 2);
    msg->PID = atoi(PID);
    printf("%i\n",msg->PID);
  
    strncat(msg->path, secondComma + 2, lastBracket - secondComma - 2);
    printf("%s\n",msg->path);
  
    return 0;
}

int writeOutFile(struct ApplicationMsg *msgs)
{
    char filePathOut[2000] = "";
    char appendToPath[] = "_out";
    char extension[] = ".txt";

    strcpy(filePathOut,msgs[0].path);
    //puntatore al carattere punto .(txt)
    char * extensionPos = strstr(filePathOut, extension);
    *extensionPos = '\0';
  
    strcat(filePathOut, appendToPath);
    strcat(filePathOut, extension);
    
  int fd = open(filePathOut, O_CREAT | O_TRUNC | O_WRONLY , S_IRUSR | S_IRUSR);

    if (fd < 0)
    {
        ErrExit("<writeMessage> Error while opening file ");
    }

    for (int i = 0; i < 4; ++i)
    {
        struct ApplicationMsg msg = msgs[i];
        char buff[MESSAGE_SIZE] = "";

        sprintf(buff, "[Parte %i, del file %s, spedita dal processo %i tramite %s]\n%s\n\n", i + 1, msg.path, msg.PID, MEDIA[msg.medium], msg.payload);

        write(fd, buff, strlen(buff));

    }

    if (close(fd) < 0)
    {
        ErrExit("<writeMessage> Error while closing file ");
    }

    return 0;
}