/// @file fs.h
/// @brief Contiene la definizioni di funzioni specifiche per la gestione del FS.

#ifndef _FS_HH
#define _FS_HH

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
 * Message format:  [<payload>, <PID>, <full path>]
 * 
 * @param filePath the file to split
 * @param msgs the messages
 * @return int
 */
int buildMessages(char *filePath,struct ApplicationMsg* msgs);

/**
 * @brief Serialize message
 *
 * @param msg the message to serialize
 * @param buff the serialized message
 * 
 * @return char *
 */
int serializeMessage(struct ApplicationMsg *msg, char *buff);

/**
 * @brief Deserialize message
 *
 * @param buff the message to deserialize
 * @param msg the message struct
 * @return struct ApplicationMsg*
 */
int deserializeMessage(char *buff, struct ApplicationMsg *msg);

#endif
