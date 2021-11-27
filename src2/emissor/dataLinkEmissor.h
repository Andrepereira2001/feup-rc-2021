#ifndef DATALINKEMISSOR_H_
#define DATALINKEMISSOR_H_

enum FrameState {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    END
};

typedef struct {
    unsigned char * frame;
    int sizeFrame;
} Frame;

void alarmCall();

int openSerialPort(char * port);

int sendMessage(int fd, unsigned char * message, unsigned size);

int receiveMessage(int fd,unsigned char * buf);

int sendControlFrame(int fd,unsigned char control);

int dataLinkState(unsigned char data, enum FrameState *frameState, int globalState, Frame * frame);

int establish();

int llopenEmissor(char * port);

int buildFrame(int fd, unsigned char * data, int size, Frame *frameBackup);

int llwrite(int fd, unsigned char* data, int dataSize);

int llclose(int fd);

#endif // DATALINK_H_
