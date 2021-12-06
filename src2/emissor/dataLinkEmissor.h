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

/**
 * @brief handles with alarm calls by sending the last message again 
 * 
 */
void alarmCall();

/**
 * @brief iniciates the communication of the serial port and sets its configurations
 * 
 * @param port - name of the serial port
 * @return int - 0 upon sucess
 */
int openSerialPort(char * port);

/**
 * @brief sends frame through the serial port and updates last message sent with the one sent
 * 
 * @param fd - serial port file descriptor
 * @param message - frame to be sent 
 * @param size - message size in bytes
 * @return int - number of bytes written in fd
 */
int sendMessage(int fd, unsigned char * message, unsigned size);

/**
 * @brief reads message from serial port
 * 
 * @param fd - serial port file descriptor
 * @param buf - to be completed with 1 bytes read
 * @return int - number of bytes read 
 */
int receiveMessage(int fd,unsigned char * buf);

/**
 * @brief builds the control frame acording to configuration
 * 
 * @param fd - file descriptor of the serial port
 * @param control - specific control flag
 * @return int - 0 upon sucess
 */
int sendControlFrame(int fd,unsigned char control);

/**
 * @brief depending on the global state of the protocol where the program is
 * it is the state machine that deals with the reception of a frame and confirms the configuration and integrity is correct
 * 
 * @param data - byte read from serial port
 * @param frameState - frame state of the recption frame state machine (configuration steps)
 * @param globalState - fase of the data link protocol (Establish, Data transfer and End)
 * @param frame - to be built with the specific config
 * @return int - 0 upon sucess
 */
int dataLinkState(unsigned char data, enum FrameState *frameState, int globalState, Frame * frame);

/**
 * @brief represents the first fase of the data link protocol 
 * where the sender sends a Set frame and waits for an UA one
 * 
 * @return int - 0 upon sucess
 */
int establish();

/**
 * @brief initiates the alarm configs, opens serial port 
 * and iniciates the data link protocol 
 * 
 * @param port - serial port path
 * @return int - file descriptor of serial port
 */
int llopenEmissor(char * port);

/**
 * @brief builds the frame to be sent with the data from the higher layer
 * executing the byte stuffing on the frame
 * 
 * @param data - information received from the higher layer to be put in the information field of the frame
 * @param size - data size in bytes
 * @param frameBackup - information frame to be filled with the data given and with the correct configuration
 * @return int - 0 upon sucess
 */
int buildFrame(unsigned char * data, int size, Frame *frameBackup);

/**
 * @brief sends frame through serial port waits for transmitter response 
 * and handles with rejection and receiver ready flags choosing which frame to send next
 *  
 * @param fd - file descriptor of the serial port
 * @param data - data received from higher layer
 * @param dataSize - data size in bytes
 * @return int - number of bytes of the frame sent
 */
int llwrite(int fd, unsigned char* data, int dataSize);

/**
 * @brief sends to the serial port the C_DISC supervision frame indicating the end of the transfering process
 * receives the C_DISC from the transmitter and sends last frame C_UA that ends the communication
 * closes the serial port at the end
 * 
 * @param fd - file descriptor of the serial port
 * @return int - 0 upon sucess
 */
int llclose(int fd);

#endif // DATALINKEMISSOR_H_
