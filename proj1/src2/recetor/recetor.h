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



/**
 * @brief parse the parcket received and does the desired actions depending on the packet state received 
 * opens a new file to write and assigns the file descriptor to the variabel fileFd (Control Packet Start received)
 * writes in file pointed by the file descritor the data that's coming in the packet (Data Packet received)
 * closes the file pointed by the file descriptor (Control Packet End received)
 *
 * @param fileFd - file descriptor of the file that is being writen
 * @param packet - packet that contains data send by the other process
 * @return int 
 */
int parsePacket(int *fileFd, AppPacket * packet);

/**
 * @brief controls the app cycle that connects the app with the data link protocol 
 * by waiting for data from the lower layer and writting it to the file
 * 
 * @param port - serial port path
 * @return int - 0 upon sucess
 */
int appFunction(char *port);

#endif // RECETOR_H_
