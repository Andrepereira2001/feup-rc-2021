#ifndef EMISSOR_H_
#define EMISSOR_H_

enum DataState {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    END
};

enum GlobalState {
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

enum PacketState {
    P_START,
    P_DATA,
    P_END
};

typedef struct {
    unsigned char *packet;
    int pSize;
    int sequenceNumber;
    enum PacketState packetState;
} AppPacket;

void alarmCall();

int dataLinkState(unsigned char data,unsigned char* word, int* curr);

int sendMessage(int fd, unsigned char * message, unsigned size);

int receiveMessage(int fd,unsigned char * buf);

int sendLatestFrame(int fd, FrameBackup *frameBackup);

int sendFrame(int fd, unsigned char * packet, int size, FrameBackup *frameBackup);

int sendControl(int fd,unsigned char control);

int createPacket(AppPacket * appPacket, unsigned char * data, int dataSize);

int readFile(int fileFd, unsigned char * buf, AppPacket * appPacket);

#endif // EMISSOR_H_
