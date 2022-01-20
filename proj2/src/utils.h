#ifndef UTILS_H_
#define UTILS_H_

#define DATA_SIZE 255

typedef struct {
    char user[DATA_SIZE];
    char password[DATA_SIZE];
    char host[DATA_SIZE];
    char path[DATA_SIZE];
    char ip[DATA_SIZE];
    char filename[DATA_SIZE];
    int port;
} Args;

/**
 * @brief parse arguments given in the command line and fill struct Args
 * 
 * @param commandArgs command line arguments
 * @param args struct to be filled
 * @return int 0 upon sucess
 */
int parseArguments(char *commandArgs,Args *args);

/**
 * @brief Get the ip
 * 
 * @param args to be filled field ip
 * @return int 0 upon sucess
 */
int getIp(Args * args);

/**
 * @brief socket configuration and connection
 * 
 * @param args with data to configure
 * @param socketfd file descriptor of the socket
 * @return int 0 upon sucess
 */
int connectSocket(Args * args, int * socketfd);

/**
 * @brief close socket
 * 
 * @param socketfd file descriptor of the socket
 * @return int 0 upon sucess
 */
int closeSocket(int *socketfd);

/**
 * @brief write command to the socketfd
 * 
 * @param socketfd file descriptor of the socket
 * @param buf buffer with commands to write in socket
 * @return int 0 upon sucess
 */
int writeSocket(int *socketfd, char * buf);

/**
 * @brief reads and shows data sent by the socket
 * 
 * @param socketfd file descriptor of the socket
 * @param buf buffer with data read from socket
 * @return int 
 */
int readSocket(int *socketfd, char * buf);

/**
 * @brief confirms if the code given was the expected to the command written
 * 
 * @param buf buffer with data read from socket
 * @param code expected confirmation c
 * @return int 0 upon sucess 1 if the code is wrong and -1 in case of error
 */
int checkCode(char * buf, char * code);

/**
 * @brief - Reads all text send until it's end
 * 
 * @param socketfd file descriptor of the socket
 * @param code expected confirmation code
 * @return int 0 upon sucess
 */
int readText(int *socketfd, char * code);

/**
 * @brief - Parse the messge received and calculates the download tcp port
 * 
 * @param buf message to be parsed
 * @return int 0 upon sucess
 */
int parsePassivePort(char * buf);

#endif // UTILS_H_