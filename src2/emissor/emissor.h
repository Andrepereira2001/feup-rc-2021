#ifndef EMISSOR_H_
#define EMISSOR_H_

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

int buildPacketStart(unsigned char * fileName, AppPacket * appPacket);

int buildPacket(unsigned char * buf, int bufSize, AppPacket * appPacket);

int buildPacketEnd(AppPacket * appPacket);

int readFromFile(int fileFd,unsigned char *buf);

int appFunction(int fileFd, char * port);

#endif // EMISSOR_H_
