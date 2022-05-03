/// @file fs.h
/// @brief Contiene la definizioni di funzioni specifiche per la gestione del FS.

#ifndef _FS_HH
#define _FS_HH

#define MAX_MESSAGE_SIZE 1024

/**
 * @brief Retrieve all the file paths matching the SENDME requirements 
 *
 * @param currdir the directory to inspect
 * @param paths the paths
 * @return int
 */
int loadFilePaths(char *currdir, char **paths);

/**
 * @brief Load the file and split it into messages
 *
 * @param filePath the file to split
 * @param buff the messages buffer
 * @return int
 */
int buildMessages(char *filePath,char** buff);

#endif
