/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "../VAR.h"
#include "recetor.h"

volatile int STOP=FALSE;
enum DataState currDataState=START;
enum GlobalState currGlobalState=ESTABLISH;

int sendMessage(int fd,unsigned char* message, int size){
    int res;
    res = write(fd,message,size);
    printf("sent: %s, %u\n",message, res);
    return 0;
}

int receiveMessage(int fd,unsigned char* buf){
    return read(fd,buf,1);
}

int sendControl(int fd, unsigned char control){

    unsigned char CONTROL[5];

    CONTROL[0]=FLAG;
    CONTROL[1]=AE_SENT;
    CONTROL[2]=control;
    CONTROL[3]=CONTROL[1]^CONTROL[2];
    CONTROL[4]=FLAG;

    sendMessage(fd, CONTROL, 5);

    return 0;
}

int destuff(unsigned char * word, int wordSize, unsigned char *packet, int *pSize){
    int i = 4;
    unsigned char bcc;
    *pSize = 0;
    while(i < wordSize){
        if(word[i]==O_ESC){
            i++;
            if(word[i]==O_FST){
                packet[*pSize]=FLAG;
            }
            else if (word[i]==O_SND){
                packet[*pSize]=O_ESC;
            }
        }
        else {
            packet[*pSize]=word[i];
        }
        i++;
        *pSize = *pSize+1;
    }

    *pSize = *pSize - 1;
    bcc = packet[0];
    for(int i=1; i<(*pSize); i++){
        bcc = packet[i]^bcc;
    }
    if(bcc != packet[*pSize]){
        return -1;
    }
    return 0;
}

int parsePacket(int *fd, unsigned char *packet, int pSize, int *sequenceNumber){
    printf("sq n : %d------------ packet 1: %d\n", *sequenceNumber, packet[1]);
    if(packet[0] == 0x01 && packet[1] == (*(sequenceNumber) + 1) % 256){
        int size = (packet[2] << 2) | packet[3];
        printf("size - %d\n", size);

        if(write(*fd,&(packet[4]),size) == -1){
            perror("Error while writing");
            return -1;
        }
        *sequenceNumber = (*sequenceNumber + 1) % 256;
        return 0;
    }
    else if (packet[0] == 0x02){
        if( packet[1] == 0x01){
            int size = packet[2];
            unsigned char fileName[255];
            for (int i = 0 ; i < size; i++){
                fileName[i] = packet[i+3];
            }
            printf("filename -> %s\n",fileName);
            *fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (*fd < 0) {printf("Erro while opening %s",fileName); return(-1);}
            *sequenceNumber = 255;
            return 0;
        }
    } else if (packet[0] == 0x03){
        close(*fd);
        return 0;
    }
    return -1;
}

int dataLinkState(unsigned char data,unsigned char* word, int* curr){
    switch(currDataState){
        case START:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            break;
        case FLAG_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(data == AE_SENT){
                currDataState = A_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case A_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(data == C_SET && (currGlobalState==TRANSFER || currGlobalState==ESTABLISH)){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else if(data == C_DISC && (currGlobalState==TRANSFER || currGlobalState==TERMINATE)){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else if(data == C_UA && currGlobalState==TERMINATE){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else if((data == C_S0 || data == C_S1) && currGlobalState == TRANSFER){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case C_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(data == word[1]^word[2]){
                currDataState = BCC_OK;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case BCC_OK:
            if(data==FLAG){
                currDataState=END;
                word[*curr]=data;
                *curr = 0;
                if (word[2] == C_SET) currGlobalState = ESTABLISH; 
                else if (currGlobalState == TRANSFER && word[2] == C_DISC){ 
                    currGlobalState = TERMINATE;
                }
                else if (currGlobalState == TERMINATE && word[2]==C_UA) currGlobalState = GLOBAL_END;
            }
            else if (currGlobalState == TRANSFER){
                currDataState = DATA_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case DATA_RCV:
            if(data==FLAG){
                currDataState=END;
                word[*curr]=data;
            }
            else {
                word[*curr]=data;
                *curr = *curr+1;
            }
            break;
        case END:
            break;
    }
}

int llread(int fd, unsigned char *word, int *wordSize){
    unsigned char buf[255];
    while (currDataState!=END) {       /* loop for input */
          receiveMessage(fd,buf);   /* returns after 5 chars have been input */
          dataLinkState(buf[0],word,wordSize);
          printf("%x ",buf[0]);
    }
    printf("\n");
    currDataState = START;

    return 0;
}

int communicate(int fd){
    //data-link variables
    unsigned char word[255];
    int curr = 0;
    int lastFrameRcv=1;

    // application variables
    unsigned char packet[255];
    int pSize=0;
    int sequenceNumber = 255;
    int fileFd;
    

    while (currGlobalState != GLOBAL_END){
        llread(fd,word,&curr);

        if(currGlobalState==TRANSFER && curr != 0){ //if it is a data frame we need the size
            int error = TRUE;
            if(destuff(word, curr, packet, &pSize) == 0){
                if(word[2] == C_S0){
                    if(lastFrameRcv == 1){
                        if (parsePacket(&fileFd, packet, pSize, &sequenceNumber) != -1){ // application level error
                            sendControl(fd, C_RR1);
                            lastFrameRcv = 0;
                            error = FALSE;
                        }
                    }
                    else if (lastFrameRcv == 0){ 
                        sendControl(fd, C_RR1);
                        lastFrameRcv = 0;
                        error = FALSE;
                    }
                }
                else if(word[2] == C_S1) {
                    if(lastFrameRcv == 0){
                        if (parsePacket(&fileFd, packet, pSize, &sequenceNumber) != -1){ // application level error
                            sendControl(fd, C_RR0);
                            lastFrameRcv = 1;
                            error = FALSE;
                        }
                    }
                    else if (lastFrameRcv == 1){ 
                        sendControl(fd, C_RR0);
                        lastFrameRcv = 1;
                        error = FALSE;
                    }
                }
            }

            if(error) {
                if(word[2] == C_S0){
                    sendControl(fd, C_REJ0);
                }
                else if(word[2] == C_S1) {
                    sendControl(fd, C_REJ1);
                }
            }
            curr = 0;
        }
        if(currGlobalState == ESTABLISH){ 
            sendControl(fd,C_UA);
            currGlobalState = TRANSFER;
        }
        else if(currGlobalState==TERMINATE) sendControl(fd,C_DISC);
    }

    return 0;
}

int main(int argc, char** argv)
{
    int fd, res;
    struct termios oldtio,newtio;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */


    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
    
    //start reading data sent
    if (communicate(fd) != 0){
        perror("communication error");
        exit(-1);
    }

    sleep(2);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
