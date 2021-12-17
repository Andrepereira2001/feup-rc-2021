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

/**
 * @brief builds the start control packet 
 * that is going to be send with the read fileName
 * 
 * @param fileName - name of the file to be sent and created by the transmitter
 * @param appPacket - packet to be completed with the start packet configurations
 * @return int - 0 upon sucess
 */
int buildPacketStart(unsigned char * fileName, AppPacket * appPacket);

/**
 * @brief copies the information received in the parameter buf 
 * to the packet to be sent and completes the packet with the configuration needed
 * 
 * @param buf - information read from file to complete the data field of the packet
 * @param bufSize - buf number of bytes
 * @param appPacket - paket to be completed with the information from buf and other configurations
 * @return int - 0 upon sucess
 */
int buildPacket(unsigned char * buf, int bufSize, AppPacket * appPacket);

/**
 * @brief builds the ending control packet 
 * with the configuration needed to determine the ending of the transfer process
 * 
 * @param appPacket - packet to be filled with the specific configuration 
 * @return int - 0 upon sucess
 */
int buildPacketEnd(AppPacket * appPacket);

/**
 * @brief fills buf with the information read from file (100 bytes)
 * 
 * @param fileFd - fd from file to be read
 * @param buf - array to be filled with the read info
 * @return int - number of bytes read
 */
int readFromFile(int fileFd,unsigned char *buf);

/**
 * @brief controls the app cycle that connects the app with the data link protocol 
 * by reading data from file and sending it to the lower layer
 * 
 * @param port - serial port path
 * @param fileName - file name of the file that will be trasnfered
 * @return int - 0 upon sucess
 */
int appFunction(char *port, char *fileName);

#endif // EMISSOR_H_
