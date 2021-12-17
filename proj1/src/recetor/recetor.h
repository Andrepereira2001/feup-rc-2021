#ifndef RECETOR_H_
#define RECETOR_H_

enum DataState{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    DATA_RCV,
    END
};

enum GlobalState{
    ESTABLISH,
    TRANSFER,
    TERMINATE,
    GLOBAL_END
};

typedef struct {
    unsigned char * latestframe;
    int sizeFrame;
    int frameToSend; // can only be 0 or 1 represent frame to be send
    int acceptedFrame;
} FrameBackup; 

int sendMessage(int fd,unsigned char* message, int size);

int receiveMessage(int fd,unsigned char* buf);

int sendControl(int fd, unsigned char control);

int destuff(unsigned char * word, int wordSize, unsigned char *packet, int *pSize);

int parsePacket(int *fd, unsigned char *packet, int pSize, int *sequenceNumber);

int dataLinkState(unsigned char data,unsigned char* word, int* curr);

int communicate(int fd);

#endif // RECETOR_H_
