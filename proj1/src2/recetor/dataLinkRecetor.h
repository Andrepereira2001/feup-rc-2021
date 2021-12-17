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

/**
 * @brief iniciates the communication of the serial port and sets its configurations
 * 
 * @param port - name of the serial port
 * @return int - 0 upon sucess
 */
int openSerialPort(char * port);

/**
 * @brief sends frame through the serial port
 * 
 * @param fd - serial port file descriptor
 * @param message - frame to be sent 
 * @param size - message size in bytes
 * @return int - number of bytes written in fd
 */
int sendMessage(int fd,unsigned char* message, int size);

/**
 * @brief reads message from serial port
 * 
 * @param fd - serial port file descriptor
 * @param buf - to be completed with 1 bytes read
 * @return int - number of bytes read 
 */
int receiveMessage(int fd,unsigned char * buf);

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
 * @brief builds the control frame acording to configuration
 * 
 * @param fd - file descriptor of the serial port
 * @param control - specific control flag
 * @return int - 0 upon sucess
 */
int sendControlFrame(int fd,unsigned char control);

/**
 * @brief - iniciate the data link protocol waiting for the C_SET supervision frame
 * and sending the C_UA supervion frame
 * 
 * @param fd - file descriptor of the serial port
 * @return int - 0 upon sucess
 */
int establish(int fd);

/**
 * @brief opens serial port 
 * and waits for the iniciation of data link protocol
 * 
 * @param port - serial port path
 * @return int - file descriptor of serial port
 */
int llopenRecetor(char* port);

/**
 * @brief destuffs the received frame verifying it's integrity recording it in data parameter
 * 
 * @param frame - frame received from the other process
 * @param data - data content of received frame (destuffed)
 * @param dataSize - size of data in bytes
 * @return int - 0 upon sucess
 */
int destuff(Frame *frame, unsigned char *data, int *dataSize);

/**
 * @brief reads frame from the serial port analyze it and send an according response
 * Receiver ready + next sequence number
 * Reject + reject frame sequence number
 * If frame received is OK the data is sent to the upper layer
 * 
 * @param fd - file descriptor of serial port
 * @param data - data content of received frame (destuffed)
 * @param dataSize - size of data in bytes
 * @return int - 0 upon sucess
 */
int llread(int fd, unsigned char *data, int *dataSize);

/**
 * @brief - receives C_DISC supervision frame indicating the end of the transfering process
 * sends back the C_DISC frame informing that the message was received upon received supervision frame C_UA
 * closes the serial port at the end
 * 
 * @param fd - file descriptor of serial port
 * @return int - 0 upon sucess
 */
int llclose(int fd);

#endif // DATALINKRECETOR_H_
