#ifndef RECETOR_H_
#define RECETOR_H_

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

int parsePacket(int *fileFd, AppPacket * packet);

int appFunction(char *port);

#endif // RECETOR_H_
