//sudo socat -d  -d  PTY,link=/dev/ttyS0,mode=777   PTY,link=/dev/ttyS1,mode=777

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "emissor.h"
#include "dataLinkEmissor.h"
#include "../VAR.h"

//unsigned char *fileName = "./pinguim1.gif";
//unsigned char *fileNameOrg = "./pinguim.gif";

int buildPacketStart(unsigned char * fileName, AppPacket * appPacket){
    appPacket->packet[0] = 0x02;
    appPacket->packet[1] = 0x01;
    appPacket->packet[2] = strlen(fileName) + 1;
    for (int i = 0; i <  strlen(fileName); i++){
        appPacket->packet[i + 3] = fileName[i];
    }
    appPacket->packet[strlen(fileName) + 3] = '\0';
    appPacket->pSize = strlen(fileName) + 4;

    appPacket->sequenceNumber = 0;
    return 0;
}

int buildPacket(unsigned char * buf, int bufSize, AppPacket * appPacket){
    appPacket->packet[0] = 0x01;
    appPacket->packet[1] = appPacket->sequenceNumber;
    appPacket->packet[2] = (0x0ff00 & bufSize) >> 2;
    appPacket->packet[3] = 0xff & bufSize;
    for(int i = 0; i < bufSize; i++){
        appPacket->packet[i+4] = buf[i];
    }

    appPacket->pSize = bufSize + 4;
    appPacket->sequenceNumber = appPacket->sequenceNumber + 1;
    return 0;
}

int buildPacketEnd(AppPacket * appPacket){
    appPacket->packet[0] = 0x03;
    appPacket->pSize = 1;
    return 0;
}

int readFromFile(int fileFd,unsigned char *buf){
    return read(fileFd,buf,100);
}

int appFunction(char *port, char *fileName){
    int fileFd;
    fileFd = open(fileName, O_RDONLY);
    if (fileFd < 0) {perror("Erro while opening test.txt"); exit(-1);}

    char finalName[255] = "clone_";

    strtok(fileName, "/");

    strcat(finalName, strtok(NULL, "/"));

    
    int fd, bufSize;
    AppPacket appPacket;
    appPacket.pSize = 0;
    appPacket.sequenceNumber = 0;
    appPacket.packetState = P_START;
    appPacket.packet = malloc (2047 * sizeof (unsigned char));

    unsigned char buf[2047];
    
    fd = llopenEmissor(port);

    while(appPacket.packetState != P_END){

        if(appPacket.packetState == P_START){
            buildPacketStart(finalName, &appPacket);
            appPacket.packetState = P_DATA;
        }else if(appPacket.packetState == P_DATA){
            bufSize = readFromFile(fileFd, buf);
            if(bufSize == 0){
                appPacket.packetState = P_END;
                printf("End of file reach\n");
                buildPacketEnd(&appPacket);
            }
            else if(bufSize == -1){
                printf("Error reading file\n");
                return -1;
            }else {
                buildPacket(buf, bufSize, &appPacket);
            }
        }

        int n = llwrite(fd, appPacket.packet , appPacket.pSize); //returns number of chars written
    }

    llclose(fd);
}

int main(int argc, char** argv){
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if (( argc < 3 )){
        printf("A file must be specifed\n");
        exit(1);
    }

    struct timeval start, end;
    double elapsedTime;
    clock_t startP, endP;

    startP = clock();
    gettimeofday(&start, NULL);

    //start sending data
    if (appFunction(argv[1], argv[2]) != 0){
        perror("communication error");
        exit(-1);
    }

    gettimeofday(&end, NULL);

    elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; 
    elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0;

    endP = clock();

    printf("Sender execution time - %f\n",elapsedTime * 1.0e-3);
    printf("Sender process execution time - %f\n",((double) (endP - startP)) / CLOCKS_PER_SEC);
    return 0;
}
