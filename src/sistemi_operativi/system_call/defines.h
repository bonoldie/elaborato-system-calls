/// @file defines.h
/// @brief Continene le definizioni delle costanti

#ifndef _DEFINES_HH
#define _DEFINES_HH

// Files
#define MAX_FILES 100
#define MAX_FILE_SIZE 4000

// Messages
#define MESSAGE_META_SIZE 300
#define MESSAGE_PAYLOAD_SIZE 4000 / 4
#define MESSAGE_SIZE MESSAGE_META_SIZE + MESSAGE_PAYLOAD_SIZE

struct ApplicationMsg
{
    pid_t PID;
    char payload[MESSAGE_PAYLOAD_SIZE];
    char path[MESSAGE_META_SIZE];
};

// Shared memory
#define SHM_KEY 123456789
#define MSG_KEY 987654321

// FIFOs
#define FIFO1PATH "/tmp/FIFO1"
#define FIFO2PATH "/tmp/FIFO2"

#endif
