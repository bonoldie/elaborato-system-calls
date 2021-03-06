/// @file defines.h
/// @brief Continene le definizioni delle costanti

#ifndef _DEFINES_HH
#define _DEFINES_HH

// Files
#define MAX_FILES 100
#define MAX_FILE_SIZE 4000

// Messages
#define MESSAGE_META_SIZE 4000
#define MESSAGE_PAYLOAD_SIZE 4000 / 4
#define MESSAGE_SIZE MESSAGE_META_SIZE + MESSAGE_PAYLOAD_SIZE

struct ApplicationMsg
{
    int PID;
    char payload[MESSAGE_PAYLOAD_SIZE];
    char path[MESSAGE_META_SIZE];
    // char medium[500];
    int medium;
};

int sortMessages(struct ApplicationMsg *msgs, int arrayLength);


// Shared memory
#define SHM_KEY 123456789
#define MSGQUEUE_KEY 987654321

// FIFOs
#define FIFO1PATH "/tmp/FIFO1"
#define FIFO2PATH "/tmp/FIFO2"

// Semaphores
#define FIFO1_PRIVATE (key_t)111111
#define FIFO2_PRIVATE (key_t)222222
#define SHM_PRIVATE (key_t)333333
#define MSGQUEUE_PRIVATE (key_t)444444
#define CLIENT_PRIVATE (key_t)555555

// MsgQueue
struct SerializedMessage {
  long mtype;
  char mtext[MESSAGE_SIZE];
};

int getMsgQueue(mode_t mode);

// Media
#define MEDIA_FIFO1_ID 0;
#define MEDIA_FIFO2_ID 1;
#define MEDIA_MSGQUEUE_ID 2;
#define MEDIA_SHM_ID 3;

extern char* MEDIA[];

#endif
