#ifndef DATALINKRECETOR_H_
#define DATALINKRECETOR_H_

enum FrameState {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    DATA_RCV,
    END
};

typedef struct {
    unsigned char * frame;
    int sizeFrame;
} Frame;

int openSerialPort(char * port);

int sendMessage(int fd,unsigned char* message, int size);

int receiveMessage(int fd,unsigned char * buf);

int dataLinkState(unsigned char data, enum FrameState *frameState, int globalState, Frame * frame);

int sendControlFrame(int fd,unsigned char control);

int establish(int fd);

int llopenRecetor(char* port);

int destuff(Frame *frame, unsigned char *data, int *dataSize);

int llread(int fd, unsigned char *data, int *dataSize);

int llclose(int fd);

#endif // DATALINKRECETOR_H_
