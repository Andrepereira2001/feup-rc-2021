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


#include "dataLinkRecetor.h"
#include "../VAR.h"
#include "recetor.h"


int parsePacket(int *fileFd, AppPacket * packet){
    printf("sq n : %d------------ packet 1: %d\n", packet->sequenceNumber, packet->packet[1]);

    //Control Packet Start
    if (packet->packet[0] == 0x02){
        if( packet->packet[1] == 0x01){
            int size = packet->packet[2];
            unsigned char fileName[255];
            for (int i = 0 ; i < size; i++){
                fileName[i] = packet->packet[i+3];
            }
            printf("filename -> %s\n",fileName);
            *fileFd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (*fileFd < 0) {printf("Erro while opening %s",fileName); return(-1);}
            packet->sequenceNumber = 255;
            packet->packetState = P_DATA;
            return 0;
        }
    } 

    //DATA Packet
    else if(packet->packet[0] == 0x01 && packet->packet[1] == (packet->sequenceNumber + 1) % 256){
        int size = (packet->packet[2] << 2) | packet->packet[3];
        printf("size - %d\n", size);

        if(write(*fileFd,&(packet->packet[4]),size) == -1){
            perror("Error while writing");
            return -1;
        }
        packet->sequenceNumber = (packet->sequenceNumber + 1) % 256;
        return 0;
    }
    
    //Control Packet End
    else if (packet->packet[0] == 0x03){
        close(*fileFd);
        packet->packetState = P_END;
        return 0;
    }
    return -1;
}

int appFunction(char *port){

    //Application
    int fd, fileFd, bufSize;
    unsigned char buf[255];

    AppPacket appPacket;
    appPacket.pSize = 0;
    appPacket.sequenceNumber = 255;
    appPacket.packetState = P_START;
    appPacket.packet = malloc (255 * sizeof (unsigned char));

    
    fd = llopenRecetor(port);

    while(appPacket.packetState != P_END){ //when we receive the end packet
        llread(fd, appPacket.packet, &appPacket.pSize);
        parsePacket(&fileFd, &appPacket);
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

    clock_t start, end;
    start = clock();

    //start sending data
    if (appFunction(argv[1]) != 0){
        perror("communication error");
        exit(-1);
    }


    end = clock();

    printf("Receiver execution time - %f",((double) (end - start)) / CLOCKS_PER_SEC);

    return 0;
}
